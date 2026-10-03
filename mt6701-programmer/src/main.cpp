/*
 * MT6701 configurator + EEPROM programmer.
 *
 * Serial-driven console. Reads live angle/status from an MT6701, lets you
 * stage a full configuration (PWM freq, direction, offset, mode), and burns
 * it to the sensor's EEPROM so the DO pin comes up in the chosen mode on
 * every subsequent power-on — no more MCU-side runtime configuration needed.
 *
 * Burn is one operation the sensor treats as final; guarded with a typed
 * "YES BURN" confirmation. Everything else is safe / reversible.
 *
 * Menu:
 *   r  read angle + field status (loop until any key)
 *   p  stage PWM output mode (default: 994.4 Hz, active-high)
 *   a  stage ABZ output mode  (asks pulses/rev)
 *   n  stage analog output mode (0-360 deg -> 0%-100% of VDD)
 *   u  stage UVW output mode  (asks pole pairs)
 *   d  toggle direction (CW <-> CCW)
 *   o  set zero offset (asks degrees)
 *   ?  print current staged config
 *   B  burn EEPROM (irreversible; requires typing YES BURN)
 *   h  help / this menu
 *
 * Wire per platformio.ini header. VDD MUST be 5 V during burn.
 */

#include <Arduino.h>
#include <Wire.h>
#include <MT6701.h>

MT6701 encoder;

// Staged config — mirrors what will be burned. NOT written to the sensor
// until you pick a mode option (which pushes via the library setter and
// therefore modifies the RAM/EEPROM shadow the sensor is using), or until
// you type `B`.
struct StagedConfig {
    enum Mode { MODE_UNSET, MODE_PWM, MODE_ABZ, MODE_UVW, MODE_ANALOG } mode = MODE_UNSET;
    mt6701_pwm_freq_t pwm_freq = MT6701_PWM_FREQ_994_4;
    mt6701_pwm_pol_t  pwm_pol  = MT6701_PWM_POL_HIGH;
    uint16_t          abz_ppr  = 1024;
    uint8_t           uvw_pp   = 15;
    float             analog_start = 0.0f;
    float             analog_stop  = 360.0f;
    mt6701_direction_t direction  = MT6701_DIRECTION_CW;
    float             offset_deg  = 0.0f;
};
static StagedConfig cfg;

static void printHelp(void);
static void printCurrentConfig(void);
static void printFieldStatus(mt6701_status_t s);
static void doRead(void);
static void doStagePWM(void);
static void doStageABZ(void);
static void doStageAnalog(void);
static void doStageUVW(void);
static void doToggleDirection(void);
static void doSetOffset(void);
static void doBurn(void);
static void skipWhitespace(void);
static long readIntFromSerial(long min_v, long max_v, long default_v);
static float readFloatFromSerial(float min_v, float max_v, float default_v);
static bool readYesBurn(void);

void setup(void)
{
    Serial.begin(115200);
    // ATmega32u4 native USB: wait for a real terminal, but not forever.
    // Otherwise if you power the Pro Micro without opening a terminal, the
    // setup blocks and it looks dead.
    unsigned long t0 = millis();
    while (!Serial && (millis() - t0) < 5000UL) { /* wait */ }

    // -------------------------------------------------------------------
    // Soft power rails for the sensor via GPIO — keeps all four jumpers on
    // the left edge of the Pro Micro (GND=A1... wait, A1 is on the right;
    // see below) and next to each other for a clean 4-wire cable. Order
    // matters for a safe power-up sequence:
    //   1. SDA/SCL pull-ups first so the I2C lines aren't floating when
    //      the sensor sees VDD.
    //   2. A1 LOW  — sensor's GND reference tied to board GND.
    //   3. D4 HIGH — sensor VDD applied.
    //   4. Delay so the sensor's internal power-on reset completes.
    //   5. Wire.begin() takes over SDA/SCL.
    //
    // Current budget (ATmega32U4 abs-max is 20 mA per pin):
    //   Read/config: ~15 mA — VOH on D4 drops to ~4.55 V, still fine (spec
    //   is VDD >= 3.0 V for normal operation).
    //   EEPROM burn: bursts to ~30 mA — VOH drops to ~4.1 V, BELOW the
    //   4.5 V burn threshold. doBurn() gates on the user swapping the VDD
    //   wire to the real VCC pin before proceeding.
    // -------------------------------------------------------------------

    // Pull-ups first (INPUT_PULLUP) — belt-and-braces, Wire.begin() also
    // does this via twi_init() but framework version drift has broken it.
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);

    // Soft-GND on A1 (right column, 7th pin from USB, across from D4).
    pinMode(A1, OUTPUT);
    digitalWrite(A1, LOW);

    // Soft-VDD on D4 (left column, 7th pin from USB, across from A1).
    // Arduino digital pins have no D-prefix constant — the silk label "D4"
    // corresponds to raw pin number 4.
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);

    // MT6701 t_startup is <30 ms per datasheet. 50 ms leaves margin.
    delay(50);

    Wire.begin();
    Wire.setClock(100000UL);   // stay at 100 kHz — internal pullups are too weak for 400 kHz

    delay(50);
    Serial.println();
    Serial.println(F("=== MT6701 configurator / EEPROM programmer ==="));
    Serial.println(F("Library: i-am-engineer/MT6701-arduino"));

    if (!encoder.initializeI2C()) {
        Serial.println(F("[FATAL] MT6701 not responding at default 0x06. Check wiring/pullups/VDD."));
        // Fall through so the menu is still usable — the user might have
        // set an alternate address, in which case they can retry.
    } else {
        Serial.println(F("MT6701 detected at 0x06."));
    }

    printHelp();
    printCurrentConfig();
    Serial.print(F("\n> "));
}

void loop(void)
{
    if (!Serial.available()) return;

    char c = Serial.read();
    // Chew any following whitespace so successive commands work.
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') return;

    Serial.println(c);

    switch (c) {
        case 'r': doRead();               break;
        case 'p': doStagePWM();           break;
        case 'a': doStageABZ();           break;
        case 'n': doStageAnalog();        break;
        case 'u': doStageUVW();           break;
        case 'd': doToggleDirection();    break;
        case 'o': doSetOffset();          break;
        case '?': printCurrentConfig();   break;
        case 'B': doBurn();               break;
        case 'h': printHelp();            break;
        default:  Serial.println(F("Unknown. Press 'h' for help.")); break;
    }
    Serial.print(F("\n> "));
}

// -----------------------------------------------------------------------
// Actions
// -----------------------------------------------------------------------

static void doRead(void)
{
    // Drain anything already in the serial buffer — the '\n' that arrived
    // with the command letter itself would otherwise trip the stop check
    // on the first loop iteration and we'd exit before ever hitting I2C.
    // Small delay first so any bytes still in transit at 115200 baud land
    // in the buffer before we drain (worst case ~90 us per char).
    delay(20);
    while (Serial.available()) Serial.read();

    Serial.println(F("Reading angle + field status. Send any character to stop."));
    while (!Serial.available()) {
        float ang = encoder.angleRead();
        mt6701_status_t st = encoder.fieldStatusRead();
        Serial.print(F("  angle="));
        Serial.print(ang, 2);
        Serial.print(F(" deg   field="));
        printFieldStatus(st);
        Serial.println();
        delay(200);
    }
    while (Serial.available()) Serial.read();   // eat the stop char + any trailing \n
    Serial.println(F("Stopped."));
}

static void doStagePWM(void)
{
    Serial.println(F("Stage PWM output mode."));
    Serial.print(F("Frequency: 1 = 497.2 Hz, 0 = 994.4 Hz [0]: "));
    long f = readIntFromSerial(0, 1, 0);
    Serial.print(F("Polarity:  0 = active-high, 1 = active-low  [0]: "));
    long p = readIntFromSerial(0, 1, 0);

    cfg.mode     = StagedConfig::MODE_PWM;
    cfg.pwm_freq = (f == 1) ? MT6701_PWM_FREQ_497_2 : MT6701_PWM_FREQ_994_4;
    cfg.pwm_pol  = (p == 1) ? MT6701_PWM_POL_LOW    : MT6701_PWM_POL_HIGH;

    encoder.pwmModeSet(cfg.pwm_freq, cfg.pwm_pol);
    encoder.directionSet(cfg.direction);
    encoder.offsetSet(cfg.offset_deg);

    Serial.println(F("Applied to sensor RAM. Use `B` to make it survive power cycles."));
    printCurrentConfig();
}

static void doStageABZ(void)
{
    Serial.println(F("Stage ABZ (quadrature) output mode."));
    Serial.print(F("Pulses per revolution [1024]: "));
    long ppr = readIntFromSerial(1, 4096, 1024);

    cfg.mode    = StagedConfig::MODE_ABZ;
    cfg.abz_ppr = (uint16_t)ppr;

    // Library defaults for pulse-width and hysteresis are sensible; expose
    // later if you need to tune them.
    encoder.abzModeSet(cfg.abz_ppr);
    encoder.directionSet(cfg.direction);
    encoder.offsetSet(cfg.offset_deg);

    Serial.println(F("Applied to sensor RAM."));
    printCurrentConfig();
}

static void doStageAnalog(void)
{
    Serial.println(F("Stage analog output mode (voltage proportional to angle)."));
    Serial.print(F("Start angle deg [0.0]: "));
    float s = readFloatFromSerial(0.0f, 360.0f, 0.0f);
    Serial.print(F("Stop angle  deg [360.0]: "));
    float e = readFloatFromSerial(0.0f, 360.0f, 360.0f);

    cfg.mode         = StagedConfig::MODE_ANALOG;
    cfg.analog_start = s;
    cfg.analog_stop  = e;

    encoder.analogModeSet(cfg.analog_start, cfg.analog_stop);
    encoder.directionSet(cfg.direction);
    encoder.offsetSet(cfg.offset_deg);

    Serial.println(F("Applied to sensor RAM."));
    printCurrentConfig();
}

static void doStageUVW(void)
{
    Serial.println(F("Stage UVW (hall emulation) output mode."));
    Serial.print(F("Pole pairs [15]: "));
    long pp = readIntFromSerial(1, 63, 15);

    cfg.mode   = StagedConfig::MODE_UVW;
    cfg.uvw_pp = (uint8_t)pp;

    encoder.uvwModeSet(cfg.uvw_pp);
    encoder.directionSet(cfg.direction);
    encoder.offsetSet(cfg.offset_deg);

    Serial.println(F("Applied to sensor RAM."));
    printCurrentConfig();
}

static void doToggleDirection(void)
{
    cfg.direction = (cfg.direction == MT6701_DIRECTION_CW)
                        ? MT6701_DIRECTION_CCW
                        : MT6701_DIRECTION_CW;
    encoder.directionSet(cfg.direction);
    Serial.print(F("Direction is now "));
    Serial.println(cfg.direction == MT6701_DIRECTION_CW ? F("CW") : F("CCW"));
}

static void doSetOffset(void)
{
    Serial.print(F("Zero offset in degrees [0.0]: "));
    cfg.offset_deg = readFloatFromSerial(-360.0f, 360.0f, 0.0f);
    encoder.offsetSet(cfg.offset_deg);
    Serial.print(F("Offset set to "));
    Serial.print(cfg.offset_deg, 2);
    Serial.println(F(" deg"));
}

static void doBurn(void)
{
    Serial.println();
    Serial.println(F("!!!  EEPROM BURN — PERMANENT  !!!"));
    Serial.println();
    Serial.println(F("### VDD wire move required BEFORE confirming ###"));
    Serial.println(F("D4 pin driver drops to ~4.1 V during burn current spikes,"));
    Serial.println(F("which is BELOW the 4.5 V EEPROM burn threshold. Move the"));
    Serial.println(F("sensor's VDD jumper from D4 to the real VCC pin (right"));
    Serial.println(F("column, 4th pin from USB) NOW, before typing anything."));
    Serial.println(F("Leave GND on A1, SDA on D2, SCL on D3 unchanged."));
    Serial.println();
    Serial.println(F("Other requirements:"));
    Serial.println(F("  1. Sensor VDD is now between 4.5 V and 5.5 V (verify multimeter)."));
    Serial.println(F("  2. Current staged config is what you want (shown below)."));
    Serial.println(F("  3. You accept finite EEPROM endurance — do not spam this."));
    Serial.println();
    printCurrentConfig();
    Serial.print(F("\nType 'YES BURN' (exactly, upper-case) then ENTER to confirm: "));

    if (!readYesBurn()) {
        Serial.println(F("Aborted — nothing written. You can move VDD back to D4 if desired."));
        return;
    }

    // Re-apply the staged config just before burn so RAM matches intent
    // even if we bounced through unrelated menu commands in between.
    switch (cfg.mode) {
        case StagedConfig::MODE_PWM:    encoder.pwmModeSet(cfg.pwm_freq, cfg.pwm_pol); break;
        case StagedConfig::MODE_ABZ:    encoder.abzModeSet(cfg.abz_ppr);               break;
        case StagedConfig::MODE_UVW:    encoder.uvwModeSet(cfg.uvw_pp);                break;
        case StagedConfig::MODE_ANALOG: encoder.analogModeSet(cfg.analog_start, cfg.analog_stop); break;
        case StagedConfig::MODE_UNSET:  /* leave whatever's in RAM */                  break;
    }
    encoder.directionSet(cfg.direction);
    encoder.offsetSet(cfg.offset_deg);

    Serial.println(F("Burning... (this can take up to a few seconds)"));
    encoder.programmEEPROM();
    Serial.println(F("Done. Move the VDD wire back to D4 if you want to keep"));
    Serial.println(F("using the compact 4-in-a-row layout. Power-cycle the sensor"));
    Serial.println(F("to verify the new default takes effect."));
}

// -----------------------------------------------------------------------
// Pretty-printers
// -----------------------------------------------------------------------

static void printFieldStatus(mt6701_status_t s)
{
    // NOTE: The MT6701's magnetic-field warning bits are only exposed via
    // the SSI interface, not I2C. The library's fieldStatusRead() reads
    // the reserved LSB bits of register 0x04, which on I2C are always 0x3
    // — the library then maps that to MT6701_STATUS_FIELD_ERROR. So an
    // "ERROR" reading here is expected and does not indicate a problem.
    // Use the actual angle-read behavior (smooth, monotonic when rotating
    // through 0 deg boundary) as the real health check.
    switch (s) {
        case MT6701_STATUS_NORM:         Serial.print(F("OK"));                break;
        case MT6701_STATUS_FIELD_STRONG: Serial.print(F("TOO STRONG"));        break;
        case MT6701_STATUS_FIELD_WEAK:   Serial.print(F("TOO WEAK"));          break;
        case MT6701_STATUS_FIELD_ERROR:  Serial.print(F("n/a via I2C — use angle sanity"));  break;
        default:                         Serial.print(F("?"));                 break;
    }
}

static void printCurrentConfig(void)
{
    Serial.println(F("Staged config:"));
    Serial.print(F("  mode      = "));
    switch (cfg.mode) {
        case StagedConfig::MODE_UNSET:  Serial.println(F("(none picked yet)")); break;
        case StagedConfig::MODE_PWM:
            Serial.print(F("PWM  freq="));
            Serial.print(cfg.pwm_freq == MT6701_PWM_FREQ_497_2 ? F("497.2 Hz") : F("994.4 Hz"));
            Serial.print(F(" pol="));
            Serial.println(cfg.pwm_pol == MT6701_PWM_POL_LOW ? F("active-low") : F("active-high"));
            break;
        case StagedConfig::MODE_ABZ:
            Serial.print(F("ABZ  ppr="));
            Serial.println(cfg.abz_ppr);
            break;
        case StagedConfig::MODE_UVW:
            Serial.print(F("UVW  pole_pairs="));
            Serial.println(cfg.uvw_pp);
            break;
        case StagedConfig::MODE_ANALOG:
            Serial.print(F("ANALOG  start="));
            Serial.print(cfg.analog_start, 2);
            Serial.print(F(" deg  stop="));
            Serial.print(cfg.analog_stop, 2);
            Serial.println(F(" deg"));
            break;
    }
    Serial.print(F("  direction = "));
    Serial.println(cfg.direction == MT6701_DIRECTION_CW ? F("CW") : F("CCW"));
    Serial.print(F("  offset    = "));
    Serial.print(cfg.offset_deg, 2);
    Serial.println(F(" deg"));
}

static void printHelp(void)
{
    Serial.println(F("\nCommands (single char, no ENTER needed):"));
    Serial.println(F("  r   read angle + field status (loop; any key to stop)"));
    Serial.println(F("  p   stage PWM output mode"));
    Serial.println(F("  a   stage ABZ output mode"));
    Serial.println(F("  n   stage analog output mode"));
    Serial.println(F("  u   stage UVW output mode"));
    Serial.println(F("  d   toggle direction CW <-> CCW"));
    Serial.println(F("  o   set zero offset"));
    Serial.println(F("  ?   print current staged config"));
    Serial.println(F("  B   burn EEPROM (irreversible; needs 'YES BURN' confirm)"));
    Serial.println(F("  h   print this help"));
}

// -----------------------------------------------------------------------
// Serial input helpers
// -----------------------------------------------------------------------

static void skipWhitespace(void)
{
    while (Serial.available()) {
        int c = Serial.peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            Serial.read();
        } else {
            break;
        }
    }
}

// Read a decimal integer terminated by newline; returns default_v on empty
// input or out-of-range. Blocks until a full line is available.
static long readIntFromSerial(long min_v, long max_v, long default_v)
{
    String s;
    unsigned long start = millis();
    while (true) {
        while (!Serial.available()) {
            if (millis() - start > 30000UL) return default_v;
        }
        char c = Serial.read();
        if (c == '\r' || c == '\n') {
            if (s.length() == 0) { Serial.println(default_v); return default_v; }
            long v = s.toInt();
            if (v < min_v || v > max_v) {
                Serial.print(F("[out of range, using ")); Serial.print(default_v); Serial.println(F("]"));
                return default_v;
            }
            Serial.println(v);
            return v;
        }
        s += c;
    }
}

static float readFloatFromSerial(float min_v, float max_v, float default_v)
{
    String s;
    unsigned long start = millis();
    while (true) {
        while (!Serial.available()) {
            if (millis() - start > 30000UL) return default_v;
        }
        char c = Serial.read();
        if (c == '\r' || c == '\n') {
            if (s.length() == 0) { Serial.println(default_v, 2); return default_v; }
            float v = s.toFloat();
            if (v < min_v || v > max_v) {
                Serial.print(F("[out of range, using ")); Serial.print(default_v, 2); Serial.println(F("]"));
                return default_v;
            }
            Serial.println(v, 2);
            return v;
        }
        s += c;
    }
}

// Reads a line and returns true iff it matches exactly "YES BURN" (no
// whitespace beyond the trailing newline).
static bool readYesBurn(void)
{
    skipWhitespace();
    String s;
    unsigned long start = millis();
    while (true) {
        while (!Serial.available()) {
            if (millis() - start > 30000UL) return false;
        }
        char c = Serial.read();
        if (c == '\r' || c == '\n') {
            Serial.println();
            return s == F("YES BURN");
        }
        s += c;
        Serial.print(c);
    }
}
