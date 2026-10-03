# MT6701 configurator / EEPROM programmer

Small PlatformIO project targeting a **SparkFun Pro Micro (ATmega32U4, 5 V /
16 MHz variant)**. Purpose: read live angle / field status from an MT6701
magnetic encoder, stage a full output-mode configuration (PWM / ABZ / UVW /
analog + direction + offset), and burn it to the sensor's EEPROM so the DO
pin comes up in the chosen mode on every subsequent power-on — no more MCU-
side runtime configuration needed.

Uses the [`I-AM-ENGINEER/MT6701-arduino`](https://github.com/I-AM-ENGINEER/MT6701-arduino)
library, which is the most complete open-source MT6701 configurator library
(supports every setter plus the EEPROM burn command).

## Why a Pro Micro instead of the STM32 already on the hoverboard board

- EEPROM programming on the MT6701 requires **4.5 V ≤ VDD ≤ 5.5 V**. The
  hoverboard board runs the sensor at 3.3 V; feeding it 5 V there would
  require touching the LDO circuit. A Pro Micro at 5 V handles this
  directly with zero rework on the main board.
- Native USB (ATmega32U4) gives an interactive serial console for typing
  confirmations before an irreversible burn.
- Compact and cheap; can be a dedicated bench tool for programming any
  MT6701 modules that arrive later.

## Wiring — compact "4-in-a-row" layout

All four sensor connections land on adjacent pins between rows 4 and 7 of
the Pro Micro. GND on the right column, SDA/SCL/VDD on the left column,
each in the same horizontal row as the next — you can plug the sensor in
with a 4-wire ribbon and it just works, no crossovers.

```
        USB
    ┌─────────┐
TX0 │ ●     ● │ RAW
RX1 │ ●     ● │ GND
GND │ ●     ● │ RST
GND │ ●     ● │ VCC   ←── swap VDD here temporarily during EEPROM burn
 D2 │ ●     ● │ A3         SDA (D2)
 D3 │ ●     ● │ A2         SCL (D3)
 D4 │ ●     ● │ A1         VDD (D4, soft-5V) ⇄ GND (A1, soft-0V)
 D5 │ ●     ● │ A0
 D6 │ ●     ● │ D15
    ...
```

| Pro Micro pin | MT6701 pin | Role                | Driven by            |
| ------------- | ---------- | ------------------- | -------------------- |
| A1            | GND        | Soft-GND (0 V)      | `digitalWrite LOW`   |
| D2            | SDA        | I2C data            | Wire library         |
| D3            | SCL        | I2C clock           | Wire library         |
| D4            | VDD        | Soft-VDD (~4.55 V)  | `digitalWrite HIGH`  |

### Why D4 is used as VDD (and why it's not the whole story)

Purely wiring ergonomics: A1 and D4 are opposite each other on rows 7, and
SDA/SCL are on rows 5/6 right above D4. All four wires plug into one board
edge in physical order (GND, SDA, SCL, VDD from top to bottom). No wire
crossings, no ribbon twists.

`setup()` runs a controlled power-up sequence: enable SDA/SCL pull-ups,
drive A1 LOW (sensor GND), drive D4 HIGH (sensor VDD), wait 50 ms for the
sensor's power-on reset, then bring up I2C.

**Current budget** — this is where the trade-off shows up:

| AVR spec / condition             | Number   |
| -------------------------------- | -------- |
| Abs-max source/sink per pin      | 20 mA    |
| VOH @ 20 mA sourcing             | ~4.4 V   |
| MT6701 typical read/write draw   | ~15 mA   |
| MT6701 EEPROM burn peak          | ~30 mA   |

At 15 mA, D4 drops from 5 V to about 4.55 V — comfortably above the
sensor's 3.0 V minimum operating VDD. Read, angle streaming, mode staging
all work fine on soft-VDD.

At 30 mA (burn spikes), D4 sags to ~4.1 V, **below** the 4.5 V EEPROM
programming threshold in the MT6701 datasheet. Burning at that voltage can
silently corrupt the EEPROM.

### Burn procedure with soft-VDD wiring

The `B` command in the console prints an explicit warning and **will not
burn until you type `YES BURN`**. Between the warning and the confirmation:

1. Physically **move the sensor's VDD jumper from D4 to the real VCC pin**
   (right column, 4th pin from USB, labeled `VCC`).
2. Verify with a multimeter that the sensor is now getting 4.8–5.0 V.
3. Then type `YES BURN`, ENTER.

Leave A1 (GND), D2 (SDA), D3 (SCL) untouched.

After the burn completes, you can move VDD back to D4 for further
verification reads. Or just power-cycle everything to confirm the sensor
comes up in the newly-burned default mode.

### Pull-ups: none required for bench use

The ATmega32U4 has internal pull-ups of ~30 kΩ that this sketch explicitly
enables on the I2C pins. At the 100 kHz bus speed configured in `setup()`
with a short jumper cable (<30 cm to the sensor module), the RC rise time
is around 6-7 µs of the 10 µs bit period — marginal but adequate for
one-shot config + burn traffic. Verified sequence:

    C_bus ≈ 100 pF   (short jumper + pin capacitance)
    τ = 30 kΩ × 100 pF = 3 µs
    t_rise ≈ 2.2τ = 6.6 µs

**Constraints to keep the internal pull-ups viable**:

1. Bus speed stays at **100 kHz** (400 kHz will not work — do not raise it
   in code).
2. Cable length **< 30 cm total**. Each extra 10 cm adds ~10 pF.
3. **One sensor at a time.** Two in parallel doubles bus capacitance.
4. Keep the cable clear of EMI sources (no motors nearby — this is a
   quiet bench setup).

If reads look noisy or Wire.endTransmission returns nonzero repeatedly,
solder **4.7 kΩ** externals from each line to 5 V and retry.

**Do NOT connect any other MCU (e.g. the hoverboard STM32) to the sensor
while burning.** Two masters on one I2C bus with different VDDs is a fast
path to a damaged chip.

### Pull-ups: none required for bench use

The ATmega32U4 has internal pull-ups of ~30 kΩ that this sketch explicitly
enables on the I2C pins. At the 100 kHz bus speed configured in `setup()`
with a short jumper cable (<30 cm to the sensor module), the RC rise time
is around 6-7 µs of the 10 µs bit period — marginal but adequate for
one-shot config + burn traffic. Verified sequence:

    C_bus ≈ 100 pF   (short jumper + pin capacitance)
    τ = 30 kΩ × 100 pF = 3 µs
    t_rise ≈ 2.2τ = 6.6 µs

**Constraints to keep the internal pull-ups viable**:

1. Bus speed stays at **100 kHz** (400 kHz will not work — do not raise it
   in code).
2. Cable length **< 30 cm total**. Each extra 10 cm adds ~10 pF.
3. **One sensor at a time.** Two in parallel doubles bus capacitance.
4. Keep the cable clear of EMI sources (no motors nearby — this is a
   quiet bench setup).

If reads look noisy or Wire.endTransmission returns nonzero repeatedly,
solder **4.7 kΩ** externals from each line to 5 V and retry.

**Do NOT connect any other MCU (e.g. the hoverboard STM32) to the sensor
while burning.** Two masters on one I2C bus with different VDDs is a fast
path to a damaged chip.

**Do NOT connect any other MCU (e.g. the hoverboard STM32) to the sensor
while burning.** Two masters on one I2C bus with different VDDs is a fast
path to a damaged chip.

## Build + upload

Requires [PlatformIO CLI](https://platformio.org/install/cli). From this
directory:

```
pio run                     # compile
pio run -t upload           # flash the Pro Micro
pio device monitor          # open the serial console
```

The Pro Micro appears as a USB CDC serial port after upload (may need to
press its reset button once if the bootloader window is missed on the first
upload).

## Console usage

Single-character commands, no ENTER after the command letter itself.
Numbers are prompted separately and terminated by ENTER.

| Key | Action                                                     |
| --- | ---------------------------------------------------------- |
| `r` | Continuously read + print angle & field status; any key stops |
| `p` | Stage PWM output mode (frequency + polarity)               |
| `a` | Stage ABZ (quadrature) mode (pulses per revolution)        |
| `n` | Stage analog mode (start & stop angle mapping to 0–100 % of VDD) |
| `u` | Stage UVW (hall emulation) mode (pole pairs)               |
| `d` | Toggle direction CW ↔ CCW                                  |
| `o` | Set zero offset in degrees                                 |
| `?` | Print current staged config                                |
| `B` | Burn EEPROM — irreversible; requires typing `YES BURN`     |
| `h` | Print help                                                 |

Every setter (`p`, `a`, `n`, `u`, `d`, `o`) applies to the sensor's RAM
immediately, so you can validate the effect with `r` before committing.
Burn writes the current RAM contents to EEPROM.

## Typical workflow to flash the hoverboard sensors to PWM output

1. Power the sensor from the Pro Micro's 5 V rail.
2. Open serial monitor (115200 baud).
3. Confirm sensor is detected — the banner should say
   `MT6701 detected at 0x06.`
4. Rotate shaft by hand, hit `r`, watch angle change smoothly, field = OK.
5. Type `p`, accept the defaults (994.4 Hz, active-high).
6. Verify with a scope on DO — you should see a ~994 Hz square wave whose
   duty cycle changes as you rotate the shaft.
7. Type `d` if the direction needs to be flipped for your mechanical setup.
8. Type `o` and enter the zero-offset (or leave it 0 and offset in firmware).
9. Type `B`, then `YES BURN`, ENTER.
10. Power-cycle the sensor. On next boot the DO pin should default to PWM
    output — no I2C init required.
