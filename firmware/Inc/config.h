// Define to prevent recursive inclusion
#ifndef CONFIG_H
#define CONFIG_H

#include "stm32f1xx_hal.h"
#include <math.h>

// ############################### BUILD VARIANT ###############################
// This project keeps only TWO_AXIS_VARIANT: two motors, MT6701 in PWM mode.
// The build define comes from platformio.ini build_flags (-D TWO_AXIS_VARIANT).
// If you need single-motor, comment ENCODER_X or ENCODER_Y further down —
// gating automatically drops the corresponding motor / driver code.


// ############################### DO-NOT-TOUCH SETTINGS ###############################
#define PWM_FREQ            16000     // PWM frequency in Hz / is also used for buzzer
#define DEAD_TIME              48     // PWM deadtime
#define DELAY_IN_MAIN_LOOP    5       // [ms] main-loop tick. Do not touch unless you know what you are doing.
#define TIMEOUT                20     // number of wrong / missing input commands before emergency off
#define A2BIT_CONV             50     // A to bit for current conversion on ADC. Example: 1 A = 50, 2 A = 100, etc
#define MANUAL_CMD_TIMEOUT_MS 150U    // streamed T/TX/TY must be renewed before this deadline
#define MANUAL_CMD_TIMEOUT_TICKS ((PWM_FREQ * MANUAL_CMD_TIMEOUT_MS + 999U) / 1000U)
_Static_assert(MANUAL_CMD_TIMEOUT_TICKS > 0U, "Manual-command deadman timeout must be positive");
// #define PRINTF_FLOAT_SUPPORT          // [-] Uncomment this for printf to support float on Serial Debug. It will increase code size! Better to avoid it!

// ADC conversion time definitions
#define ADC_CONV_TIME_1C5       (14)  //Total ADC clock cycles / conversion = (  1.5+12.5)
#define ADC_CONV_TIME_7C5       (20)  //Total ADC clock cycles / conversion = (  7.5+12.5)
#define ADC_CONV_TIME_13C5      (26)  //Total ADC clock cycles / conversion = ( 13.5+12.5)
#define ADC_CONV_TIME_28C5      (41)  //Total ADC clock cycles / conversion = ( 28.5+12.5)
#define ADC_CONV_TIME_41C5      (54)  //Total ADC clock cycles / conversion = ( 41.5+12.5)
#define ADC_CONV_TIME_55C5      (68)  //Total ADC clock cycles / conversion = ( 55.5+12.5)
#define ADC_CONV_TIME_71C5      (84)  //Total ADC clock cycles / conversion = ( 71.5+12.5)
#define ADC_CONV_TIME_239C5     (252) //Total ADC clock cycles / conversion = (239.5+12.5)

// This settings influences the actual sample-time. Only use definitions above
// This parameter needs to be the same as the ADC conversion for Current Phase of the FIRST Motor in setup.c
#define ADC_CONV_CLOCK_CYCLES   (ADC_CONV_TIME_7C5)

// Set the configured ADC divider. This parameter needs to be the same ADC divider as PeriphClkInit.AdcClockSelection (see main.c)
#define ADC_CLOCK_DIV           (4)

// ADC Total conversion time: this will be used to offset TIM8 in advance of TIM1 to align the Phase current ADC measurement
// This parameter is used in setup.c
#define ADC_TOTAL_CONV_TIME     (ADC_CLOCK_DIV * ADC_CONV_CLOCK_CYCLES) // = ((SystemCoreClock / ADC_CLOCK_HZ) * ADC_CONV_CLOCK_CYCLES), where ADC_CLOCK_HZ = SystemCoreClock/ADC_CLOCK_DIV

// Current-sense offset calibration (phase/DC-link ADC centers)
#define CURRENT_SENSE_OFFSET_INIT             2048
#define CURRENT_SENSE_OFFSET_CAL_SAMPLES      2000U
#define CURRENT_SENSE_OFFSET_CAL_MODE_AVG
#define CURRENT_SENSE_OFFSET_DCL_TRIM         0     // [ADC counts] Left DC-link offset trim added after calibration average
#define CURRENT_SENSE_OFFSET_DCR_TRIM         0     // [ADC counts] Right DC-link offset trim added after calibration average
// ########################### END OF  DO-NOT-TOUCH SETTINGS ############################

// ############################### BOARD VARIANT ###############################
/* Board Variant
 * 0 - Default board type
 * 1 - Alternate board type with different pin mapping for DCLINK, Buzzer and ON/OFF, Button and Charger
*/
#define BOARD_VARIANT           0         // change if board with alternate pin mapping
#define GD32F103Rx              1   // define if you are using a GD32F103Rx MCU to set system clock to 108MHz  
// ######################## END OF BOARD VARIANT ###############################

// ############################### BATTERY ###############################
/* Battery voltage calibration: connect power source.
 * see How to calibrate.
 * Write debug output value nr 5 to BAT_CALIB_ADC. make and flash firmware.
 * Then you can verify voltage on debug output value 6 (to get calibrated voltage multiplied by 100).
*/
#define BAT_FILT_COEF           16384       // battery voltage filter coefficient in fixed-point. coef_fixedPoint = coef_floatingPoint * 2^16. In this case 16384 = 0.25 * 2^16
#define BAT_CALIB_REAL_VOLTAGE  3970      // input voltage measured by multimeter (multiplied by 100). In this case 43.00 V * 100 = 4300
#define BAT_CALIB_ADC           1492      // adc-value measured by mainboard (value nr 5 on UART debug output)
#define BAT_ADC_TO_CV(adc)      ((int16_t)((((int32_t)(adc) * BAT_CALIB_REAL_VOLTAGE) + (BAT_CALIB_ADC / 2)) / BAT_CALIB_ADC))
#define BAT_CELLS               6       // battery number of cells. Normal Hoverboard battery: 10s = 36V nominal, 42V full charge. For 36V battery use 10, for 24V use 6, for 48V use 13 etc.
#define BAT_LVL2_ENABLE         0         // to beep or not to beep, 1 or 0
#define BAT_LVL1_ENABLE         0         // to beep or not to beep, 1 or 0
#define BAT_DEAD_ENABLE         1         // to poweroff or not to poweroff, 1 or 0
#define BAT_BLINK_INTERVAL      80        // battery led blink interval (80 loops * 5ms ~= 400ms)
#define BAT_HIGH                (550 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE;
#define BAT_LVL5                (390 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // Green blink:  no beep
#define BAT_LVL4                (380 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // Yellow:       no beep
#define BAT_LVL3                (370 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // Yellow blink: no beep 
#define BAT_LVL2                (360 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // Red:          gently beep at this voltage level. [V*100/cell]. In this case 3.60 V/cell
#define BAT_LVL1                (350 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // Red blink:    fast beep. Your battery is almost empty. Charge now! [V*100/cell]. In this case 3.50 V/cell
#define BAT_DEAD                (330 * BAT_CELLS * BAT_CALIB_ADC) / BAT_CALIB_REAL_VOLTAGE    // All leds off: undervoltage poweroff. (while not driving) [V*100/cell]. In this case 3.37 V/cell
#define HARD_LIM                 1800  //hard 18v lower limit to prevent damage to driver
#define HARD_18V_COUNTS (((HARD_LIM) * BAT_CALIB_ADC + (BAT_CALIB_REAL_VOLTAGE / 2)) / (BAT_CALIB_REAL_VOLTAGE))
// ######################## END OF BATTERY ###############################



// ############################### TEMPERATURE ###############################
/* Board overheat detection: the sensor is inside the STM/GD chip.
 * It is very inaccurate without calibration (up to 45°C). So only enable this funcion after calibration!
 * Let your board cool down.
 * see <How to calibrate.
 * Get the real temp of the chip by thermo cam or another temp-sensor taped on top of the chip and write it to TEMP_CAL_LOW_DEG_C.
 * Write debug output value 8 to TEMP_CAL_LOW_ADC. drive around to warm up the board. it should be at least 20°C warmer. repeat it for the HIGH-values.
 * Enable warning and/or poweroff and make and flash firmware.
*/
// Comment out ENABLE_BOARD_TEMP_SENSOR to skip measuring the MCU temperature sensor.
//#define ENABLE_BOARD_TEMP_SENSOR
#define TEMP_FILT_COEF          655       // temperature filter coefficient in fixed-point. coefFixedPoint = coef_floatingPoint * 2^16. In this case 655 = 0.01 * 2^16
#define TEMP_CAL_LOW_ADC        1655      // temperature 1: ADC value
#define TEMP_CAL_LOW_DEG_C      358       // temperature 1: measured temperature [°C * 10]. Here 35.8 °C
#define TEMP_CAL_HIGH_ADC       1588      // temperature 2: ADC value
#define TEMP_CAL_HIGH_DEG_C     489       // temperature 2: measured temperature [°C * 10]. Here 48.9 °C
#define TEMP_WARNING_ENABLE     0         // to beep or not to beep, 1 or 0, DO NOT ACTIVITE WITHOUT CALIBRATION!
#define TEMP_WARNING            600       // annoying fast beeps [°C * 10].  Here 60.0 °C
#define TEMP_POWEROFF_ENABLE    0         // to poweroff or not to poweroff, 1 or 0, DO NOT ACTIVITE WITHOUT CALIBRATION!
#define TEMP_POWEROFF           650       // overheat poweroff. (while not driving) [°C * 10]. Here 65.0 °C
// ######################## END OF TEMPERATURE ###############################


// ############################### DC LINK WATCHDOG ###############################
//watchdog for over or under voltage protection
//#define DC_LINK_WATCHDOG_ENABLE
// ######################## END OF DC LINK WATCHDOG ###############################



// ############################### MOTOR CONTROL #########################
/* GENERAL NOTES:
 * 1. The parameters here are over-writing the default motor parameters. For all the available parameters check BLDC_controller_data.c
 * 2. The parameters are represented in fixed point data type for a more efficient code execution
 * 3. For calibrating the fixed-point parameters use the Fixed-Point Viewer tool (see <https://github.com/EmanuelFeru/FixedPointViewer>)
 * 4. For more details regarding the parameters and the working principle of the controller please consult the Simulink model
 * 5. A webview was created, so Matlab/Simulink installation is not needed, unless you want to regenerate the code.
 * The webview is an html page that can be opened with browsers like: Microsoft Internet Explorer or Microsoft Edge
 *
 * NOTES Field Weakening / Phase Advance:
 * 1. The Field Weakening is a linear interpolation from 0 to FIELD_WEAK_MAX or PHASE_ADV_MAX (depeding if FOC or SIN is selected, respectively)
 * 2. The Field Weakening starts engaging at FIELD_WEAK_LO and reaches the maximum value at FIELD_WEAK_HI
 * 3. If you re-calibrate the Field Weakening please take all the safety measures! The motors can spin very fast!

  Inputs:
   - input1[inIdx].cmd and input2[inIdx].cmd: normalized input values. INPUT_MIN to INPUT_MAX
   - button1 and button2: digital input values. 0 or 1
   - adc_buffer.adc3.value.l_tx2 and adc_buffer.adc3.value.l_rx2: unfiltered ADC values (you do not need them). 0 to 4095
   Outputs:
    - cmdL and cmdR: normal driving INPUT_MIN to INPUT_MAX
*/
#define COM_CTRL        0               // [-] Commutation Control Type
#define SIN_CTRL        1               // [-] Sinusoidal Control Type
#define FOC_CTRL        2               // [-] Field Oriented Control (FOC) Type


#define CFG_OPEN_MODE   0               // [-] OPEN mode
#define CFG_VLT_MODE    1               // [-] VOLTAGE mode
#define CFG_SPD_MODE    2               // [-] SPEED mode
#define CFG_TRQ_MODE    3               // [-] TORQUE mode

// Enable/Disable Motor
#define MOTOR_LEFT_ENA                  // [-] Enable LEFT motor.  Comment-out if this motor is not needed to be operational
#define MOTOR_RIGHT_ENA                 // [-] Enable RIGHT motor. Comment-out if this motor is not needed to be operational

// Control selections
#define CTRL_TYP_SEL    FOC_CTRL        // [-] Control type selection: COM_CTRL, SIN_CTRL, FOC_CTRL (default)
#define CTRL_MOD_REQ    CFG_TRQ_MODE    // [-] Control mode request: CFG_OPEN_MODE, CFG_VLT_MODE (default), CFG_SPD_MODE, CFG_TRQ_MODE. Note: CFG_SPD_MODE and CFG_TRQ_MODE are only available for CTRL_FOC!
#define DIAG_ENA        0              // [-] Motor Diagnostics enable flag: 0 = Disabled, 1 = Enabled (default)

// Limitation settings
#define I_MOT_MAX       15              // [A] Maximum single motor current limit
#define I_DC_MAX        17              // [A] Maximum stage2 DC Link current limit for Commutation and Sinusoidal types (This is the final current protection. Above this value, current chopping is applied. To avoid this make sure that I_DC_MAX = I_MOT_MAX + 2A)
#define I_DCL_POS       20              // [A]    LEFT Positive DC link current protection limit. Above this value, the power stage is disabled. 
#define I_DCL_NEG       20              // [A]    LEFT Negative DC link current protection limit. Below this value, the power stage is disabled.
#define I_DCR_POS       20              // [A]    RIGHT Positive DC link current protection limit. Above this value, the power stage is disabled.  
#define I_DCR_NEG       20              // [A]    RIGHT Negative DC link current protection limit. Below this value, the power stage is disabled.
#define DCL_HIGH_COUNTS  (2000 + (I_DCL_POS * A2BIT_CONV))  // compile-time left high threshold in ADC counts (nominal offset 2000)
#define DCL_LOW_COUNTS   (2000 - (I_DCL_NEG * A2BIT_CONV))  // compile-time left low threshold in ADC counts (nominal offset 2000)
#define DCR_HIGH_COUNTS  (2000 + (I_DCR_POS * A2BIT_CONV))  // compile-time right high threshold in ADC counts (nominal offset 2000)
#define DCR_LOW_COUNTS   (2000 - (I_DCR_NEG * A2BIT_CONV))  // compile-time right low threshold in ADC counts (nominal offset 2000)
#define N_MOT_MAX       200             // [rpm] Maximum motor speed limit
#define N_POLE_PAIRS    15                //[PP] Number of motor pole pairs: 15 for standard Hoverboard motors

// Field Weakening / Phase Advance
#define FIELD_WEAK_ENA  0               // [-] Field Weakening / Phase Advance enable flag: 0 = Disabled (default), 1 = Enabled
#define FIELD_WEAK_MAX  5               // [A] Maximum Field Weakening D axis current (only for FOC). Higher current results in higher maximum speed. Up to 10A has been tested using 10" wheels.
#define PHASE_ADV_MAX   25              // [deg] Maximum Phase Advance angle (only for SIN). Higher angle results in higher maximum speed.
#define FIELD_WEAK_HI   1000            // (1000, 1500] Input target High threshold for reaching maximum Field Weakening / Phase Advance. Do NOT set this higher than 1500.
#define FIELD_WEAK_LO   750             // ( 500, 1000] Input target Low threshold for starting Field Weakening / Phase Advance. Do NOT set this higher than 1000.

//Q axis control gains
#define QP            0.3f                                  //[-] P gain
#define QI            100.0f                                //[-] I gain

//D axis control gains
#define DP            0.2f                                  //[-] P gain
#define DI            50.0f                                 //[-] I gain
/* BLDC gain parameter bridge (fixed-point model)
 * Compile-time conversion from float tuning values to fixed-point parameters.
 */

/* Compile-time float -> fixed-point conversion helpers */
#define FIXDT_ROUND_TO_INT(x)        ((int32_t)(((x) >= 0.0f) ? ((x) + 0.5f) : ((x) - 0.5f)))
#define FIXDT_FROM_FLOAT(x, frac)    FIXDT_ROUND_TO_INT((x) * (float)(1U << (frac)))
#define FIXDT_CLAMP_U16(x)           ((uint16_t)(((x) < 0) ? 0 : (((x) > 65535) ? 65535 : (x))))
#define FIXDT_CLAMP_S16(x)           ((int16_t)(((x) < -32768) ? -32768 : (((x) > 32767) ? 32767 : (x))))

//#define BEEPER_OFF
//#define ENCODER_X
//#define ENCODER_Y                         // 
#define ANALOG_BUTTON                     // ANALOG BUTTON_PIN supports greater range of voltage levels.
//#define HOCP                            // Tie PA6/PB12 hardware over-current signals into TIM1/TIM8 break inputs
// #define STANDSTILL_HOLD_ENABLE          // [-] Flag to hold the position when standtill is reached. Only available and makes sense for VOLTAGE or TORQUE mode.
// #define ELECTRIC_BRAKE_ENABLE           // [-] Flag to enable electric brake and replace the motor "freewheel" with a constant braking when the input torque request is 0. Only available and makes sense for TORQUE mode.
// #define ELECTRIC_BRAKE_MAX    100       // (0, 500) Maximum electric brake to be applied when input torque request is 0 (pedal fully released).
// #define ELECTRIC_BRAKE_THRES  120       // (0, 500) Threshold below at which the electric brake starts engaging.
// ########################### END OF MOTOR CONTROL ########################



// ############################## DEFAULT SETTINGS ############################
// Default settings will be applied at the end of this config file if not set before
#define INACTIVITY_TIMEOUT        8      // Minutes of not driving until poweroff. it is not very precise. Set to 0 to deactivate
#define BEEPS_BACKWARD            0       // 0 or 1
#define ADC_MARGIN                100     // ADC input margin applied on the raw ADC min and max to make sure the MIN and MAX values are reached even in the presence of noise
#define ADC_PROTECT_TIMEOUT       100     // ADC Protection: number of wrong / missing input commands before safety state is taken
#define ADC_PROTECT_THRESH        200     // ADC Protection threshold below/above the MIN/MAX ADC values
//#define AUTO_CALIBRATION_ENA            // [DISABLED] Was for analog joystick/pot inputs. TWO_AXIS uses serial only.

/* FILTER is in fixdt(0,16,16): VAL_fixedPoint = VAL_floatingPoint * 2^16. In this case 6553 = 0.1 * 2^16
 * Value of COEFFICIENT is in fixdt(1,16,14)
 * If VAL_floatingPoint >= 0, VAL_fixedPoint = VAL_floatingPoint * 2^14
 * If VAL_floatingPoint < 0,  VAL_fixedPoint = 2^16 + floor(VAL_floatingPoint * 2^14).
*/
// Value of RATE is in fixdt(1,16,4): VAL_fixedPoint = VAL_floatingPoint * 2^4. In this case 480 = 30 * 2^4
#define DEFAULT_RATE                480   // 30.0f [-] lower value == slower rate [0, 32767] = [0.0, 2047.9375]. Do NOT make rate negative (>32767)
#define DEFAULT_FILTER              6553  // Default for FILTER 0.1f [-] lower value == softer filter [0, 65535] = [0.0 - 1.0].
#define DEFAULT_SPEED_COEFFICIENT   16384 // Default for SPEED_COEFFICIENT 1.0f [-] higher value == stronger. [0, 65535] = [-2.0 - 2.0]. In this case 16384 = 1.0 * 2^14
#define DEFAULT_STEER_COEFFICIENT   8192  // Defualt for STEER_COEFFICIENT 0.5f [-] higher value == stronger. [0, 65535] = [-2.0 - 2.0]. In this case  8192 = 0.5 * 2^14. If you do not want any steering, set it to 0.
// ######################### END OF DEFAULT SETTINGS ##########################



// ############################## INPUT FORMAT ############################
/* ***_INPUT: TYPE, MIN, MID, MAX, DEADBAND
 * -----------------------------------------
 * TYPE:      0:Disabled, 1:Normal Pot, 2:Middle Resting Pot, 3:Auto-detect
 * MIN:       min ADC3-value while poti at minimum-position (0 - 4095)
 * MID:       mid ADC3-value while poti at mid-position (INPUT_MIN - INPUT_MAX)
 * MAX:       max ADC3-value while poti at maximum-position (0 - 4095)
 * DEADBAND:  how much of the center position is considered 'center' (100 = values -100 to 100 are considered 0)
 * 
 * Dual-inputs
 * PRI_INPUT: Primary   Input. These limits will be used for the input with priority 0
 * AUX_INPUT: Auxiliary Input. These limits will be used for the input with priority 1
 * -----------------------------------------
*/
 // ############################## END OF INPUT FORMAT ############################



// ############################## CRUISE CONTROL SETTINGS ############################
/* Cruise Control info:
 * enable CRUISE_CONTROL_SUPPORT and (SUPPORT_BUTTONS_LEFT or SUPPORT_BUTTONS_RIGHT depending on which cable is the button installed)
 * can be activated/deactivated by pressing button1 (Blue cable) to GND
 * when activated, it maintains the current speed by switching to SPD_MODE. Acceleration is still possible via the input request, but when released it resumes to previous set speed.
 * when deactivated, it returns to previous control MODE and follows the input request.
*/
// #define CRUISE_CONTROL_SUPPORT
// #define SUPPORT_BUTTONS_LEFT              // Use button1 (Blue Left cable)  to activate/deactivate Cruise Control
// #define SUPPORT_BUTTONS_RIGHT             // Use button1 (Blue Right cable) to activate/deactivate Cruise Control

// ######################### END OF CRUISE CONTROL SETTINGS ##########################



// ############################### DEBUG SERIAL ###############################
/* Connect GND and RX of a 3.3v uart-usb adapter to the left (USART2) or right sensor board cable (USART3)
 * Be careful not to use the red wire of the cable. 15v will destroy everything.
 * enable DEBUG_SERIAL_USART3 or DEBUG_SERIAL_USART2
 *
 *
 * DEBUG ASCII output is:
 * // "in1:345 in2:1337 cmdL:0 cmdR:0 BatADC:0 BatV:0 TempADC:0 Temp:0\r\n"
 *
 * in1:     (int16_t)input1[inIdx].raw);                                        raw input1: ADC3, UART, PWM, PPM, iBUS
 * in2:     (int16_t)input2[inIdx].raw);                                        raw input2: ADC3, UART, PWM, PPM, iBUS
 * cmdL:    (int16_t)cmdL);                                                     output command Left: [-1000, 1000]
 * cmdR:    (int16_t)cmdR);                                                     output command Right: [-1000, 1000]
 * BatADC:  (int16_t)adc_buffer.adc3.value.batt1);                             Battery adc-value measured by mainboard
 * BatV:    (int16_t)(batVoltage * BAT_CALIB_REAL_VOLTAGE / BAT_CALIB_ADC));    Battery calibrated voltage multiplied by 100 for verifying battery voltage calibration
 * TempADC: (int16_t)board_temp_adcFilt);                                       for board temperature calibration
 * Temp:    (int16_t)board_temp_deg_c);                                         Temperature in celcius for verifying board temperature calibration
 *
*/

// #define DEBUG_SERIAL_USART2          // left sensor board cable, disable if ADC or PPM is used!
// #define DEBUG_SERIAL_USART3          // right sensor board cable, disable if I2C (nunchuk or lcd) is used!
// #define DEBUG_SERIAL_PROTOCOL        // uncomment this to send user commands to the board, change parameters and print specific signals (see comms.c for the user commands)
// ########################### END OF DEBUG SERIAL ############################



// ############################### DEBUG LCD ###############################
// #define DEBUG_I2C_LCD                // standard 16x2 or larger text-lcd via i2c-converter on right sensor board cable
// ########################### END OF DEBUG LCD ############################



#ifdef TWO_AXIS_VARIANT
/* ###### CONTROL VIA ENCODER ######
 * This variant is for using encoders for motor control.
*/
// undefine macros that will be reconfigured for the two axis variant
#undef MOTOR_LEFT_ENA
#undef CTRL_MOD_REQ
#undef CTRL_TYP_SEL
#undef DIAG_ENA
#undef BAT_CELLS
#undef INACTIVITY_TIMEOUT
#undef N_MOT_MAX
#undef I_MOT_MAX
#undef I_DC_MAX
#undef FIELD_WEAK_ENA
#undef QP
#undef QI
#undef DP
#undef DI
////////////////////////////////////////

#define GD32F103Rx              1   // define if you are using a GD32F103Rx MCU to set system clock to 108MHz
#define HOCP                        // Tie PA6/PB12 hardware over-current signals into TIM1/TIM8 break inputs
//#define BEEPER_OFF                //use led as beeper — disabled: physical buzzer on PA4 will be driven
#define ENCODER_X                 // [DISABLED — testing Y side only]
#define ENCODER_Y                   // motor Y (physically on R_MTR / TIM8 side of the board)
//#define INTBRK_L_EN               //enable brake resistor control on PHASE A left side driver, do not disable if break reistor is connected 
#define EXTBRK_EN                   // enable brake resistor control pin on left uart port, pick PA2 or PA3 below
#define CFG_USE_BW_PI_CALC
#ifdef EXTBRK_EN                         
#define EXTBRK_USE_CH3              // PA2      
//#define EXTBRK_USE_CH4            // PA3
#endif

#if defined (INTBRK_L_EN) || defined (EXTBRK_EN)

  #define BRK_VOLTAGE_RAMP_ENABLED          // Uncomment to enable voltage-based brake resistor fallback ramp
  #define BRAKE_RESISTANCE 300                // [Ohm]3ohm X100 Value of the braking resistor. Set it to your own brake resistor resistance, increase the resistance here a bit for example I use 2.2ohm but I set to 3ohm here to be safe. 
  #define BRKRESACT_SENS    40 / 20           //[A]40mA  Brake resistor activation sensitivity. Set same as MAX_REGEN_CURRENT if using battery. If using psu set 40mA-60mA. 
  #define MAX_REGEN_CURRENT 0 / 20            // [A]0mA  Maximum regenerative current that can be dissipated in the PSU or BATTERY. Set in 20mA steps 0, 20, 40, 60, 80, 100 etc. Set 0 for PSU!
  #define BRK_OVERVOLTAGE_RAMP_START (BAT_CELLS * 410) // [V*100] Voltage fallback starts adding brake duty (default 4.10 V/cell)
  #define BRK_OVERVOLTAGE_RAMP_END   (BAT_CELLS * 430) // [V*100] Voltage fallback reaches full extra duty ramp (default 4.30 V/cell)

#endif  

#define  ENCODER_IC_FILTER       6

/* CRITICAL: ENCODER_CPR must be defined so BLDC_controller gets a_cpr/a_fcpr.
 * If undefined, a_cpr=0 and the FOC cannot map encoder counts to electrical
 * angle — currents flow into wrong axes and no net torque is produced.
 * PWM mode delivers a 12-bit angle (4096 steps/rev), so the whole count
 * space is 4096 — one consistent scale, no fake resolution. (I2C mode was
 * 14-bit / 16384; if you ever revert an axis to I2C, this and MT6701_CPR in
 * mt6701.h must go back to 16384.) */
#define ENCODER_CPR              4096      // MT6701 PWM 12-bit resolution

/* MT6701 I2C bus clock (Hz). Applied to both I2C1 and I2C2 in setup.c.
 *   100000 = standard mode  — tolerante a pull-ups fracos, útil pra debug
 *   400000 = fast mode      — produção com pull-ups 2k2-4k7                       */
#define MT6701_I2C_CLOCK_HZ      100000

/* Rate at which TIM7 IRQ polls the MT6701 sensors (both X and Y each tick).
 * Fed into the FOC ISR via encoder_?.ENCODER_COUNT. Higher = smoother torque
 * but shorter tick budget for the I2C reads (~200 µs each at 200 kHz).
 * At 1000 Hz we use ~40-80% of the tick for two reads — safe margin. */
#define MT6701_SAMPLE_RATE_HZ    1000

/* PWM mode = 12-bit (4096 counts/rev). ENCODER_?_CPR = PPR*4 (see defines.h),
 * so PPR = 1024 gives CPR = 4096, matching MT6701_CPR and ENCODER_CPR. */
#if defined ENCODER_X
#define ENCODER_X_PPR            1024       // CPR = 4096 (MT6701 PWM 12-bit)
#define ALIGNMENT_X_POWER        3000       // Git-original value
#endif
#if defined ENCODER_Y
#define ENCODER_Y_PPR            1024       // CPR = 4096 (MT6701 PWM 12-bit)
#define ALIGNMENT_Y_POWER        3000       // Git-original value
#endif

#define BAT_CELLS               10      // 10s = 36V nominal, 42V full charge (bring-up battery)
#ifdef CFG_USE_BW_PI_CALC
/*
 * D/Q current-loop PI tuning
 * - Default: use manual QP/QI/DP/DI values.
 * - Optional: define CFG_USE_BW_PI_CALC to auto-compute gains from bandwidth, L, R, and VBUS.
 */
#define CFG_TARGET_BANDWIDTH_HZ   300.0f                 // [Hz] reverted to default
//#define CFG_MOTOR_L_H             0.0003f  // [H] phase-to-neutral inductance (one phase)
//#define CFG_MOTOR_R_OHM           0.3f    // [Ohm] phase-to-neutral resistance (one phase)
#define CFG_MOTOR_L_H             0.000088f  // [H] phase inductance 
#define CFG_MOTOR_R_OHM           0.157863f
#define CFG_CURR_FILT_TARGET_MULT 3U    // [-] target multiplier for current filter cutoff frequency relative to bandwidth. Higher value == softer filter. Recommended: 3+.
#else
//Q axis control gains
#define QP            0.3f                                  //[-] P gain
#define QI            100.0f                                //[-] I gain
//D axis control gains
#define DP            0.2f                                  //[-] P gain
#define DI            50.0f                                 //[-] I gain
#define CFG_CURR_FILT                0.12f
#define CFG_CF_CURR_FILT             FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(CFG_CURR_FILT, 16))
#endif

  #define FLASH_WRITE_KEY        0x1012    // Flash memory writing key.
  #define CTRL_TYP_SEL           FOC_CTRL
  #define CTRL_MOD_REQ           CFG_TRQ_MODE   // back to torque mode

  /* STATIC_ALIGN disabled: use git-original rotating alignment sequence */
  //#define STATIC_ALIGN
  
  #define TANK_STEERING
/* Only enable driver for motor whose encoder is present. Reduces EMI on I2C
 * bus and eliminates parasitic 50%-duty PWM on the non-tested motor phases. */
#ifdef ENCODER_Y
#define MOTOR_LEFT_ENA                  // TIM8 drives physical R_MTR connector
#endif
#ifdef ENCODER_X
#define MOTOR_RIGHT_ENA                 // TIM1 drives physical L_MTR connector
#endif
#define DIAG_ENA                 0               // [-] Motor Diagnostics enable flag: 0 = Disabled, 1 = Enabled (default)

/* === BRING_UP_MODE ============================================================
 * Enable while testing on the bench WITHOUT proper supply voltage / motors / encoders.
 * Disables the safety auto-poweroff paths that fire when bat voltage / DC link is low
 * (which would otherwise trigger poweroff() and leave the firmware in while(1)
 * with the buzzer stuck at the last melody tone — looking like a hard freeze).
 * REMOVE THIS DEFINE BEFORE FIELD USE so battery/DC protections come back. */
#define BRING_UP_MODE

#ifdef BRING_UP_MODE
  #undef  BAT_DEAD_ENABLE
  #define BAT_DEAD_ENABLE        0
  #define INACTIVITY_TIMEOUT     0     // 0 = disabled (no inactivity poweroff)
  /* Skip auto-alignment while validating the bare-metal I2C1 lib. Rotate the
   * Y shaft by hand and watch Y:raw= change smoothly at ~1 kHz.
   * Re-enabled now that the bare-metal driver is confirmed reading at ~1.2 kHz
   * with zero errors. */
  //#define SKIP_AUTO_ALIGN
  /* Use the bare-metal state-machine driver for I2C1 / MT6701 Y. Replaces the
   * HAL_I2C_Mem_Read_DMA path (which polls ~10 flags inside every KickIT ->
   * ~200-500 us blocked in the TIM7 IRQ) with an interrupt-only state machine
   * that only touches CR1/DR/SR1/SR2 registers directly. No HAL calls, no
   * polling, no main-loop recovery for Y. See Src/mt6701.c. */
  #define MT6701_BAREMETAL_Y
  /* Same driver mirrored for I2C2 / MT6701 X. Both encoders now use the
   * bare-metal state machine — each has its own state, counters, and
   * recovery flag. See MT6701_X_Bare_* in Src/mt6701.c. */
  #define MT6701_BAREMETAL_X

  /* MT6701 sensors configured (via bench-side EEPROM burn) to output PWM
   * on their OUT pin — no I2C runtime required. Each axis free-runs on a
   * timer input capture:
   *   X: OUT -> PB10 (TIM2_CH3)
   *   Y: OUT -> PB6  (TIM4_CH1)
   * Wire each sensor's OUT pin directly to its capture pin; the old I2C
   * SDA/SCL wiring is no longer used. Angle is the native 12-bit value
   * (0..4095), which is why ENCODER_?_CPR / MT6701_CPR are all 4096 below.
   * Gated on ENCODER_X / ENCODER_Y so that disabling an axis (commenting
   * its ENCODER_? define up top) cleanly removes ALL of that axis's PWM
   * code — timer init, IRQ handler, feed, diagnostics — with no dangling
   * references. Comment an axis's ENCODER_? define to disable it entirely. */
  #ifdef ENCODER_X
    #define MT6701_MODE_PWM_X
  #endif
  #ifdef ENCODER_Y
    #define MT6701_MODE_PWM_Y
  #endif
#else
  #define INACTIVITY_TIMEOUT     100
#endif

// Limitation settings
/* Raised for higher torque (was 8/10). Phase current (Iq) saturates at
 * I_MOT_MAX in the FOC; I_DC_MAX is the level-2 DC-link overcurrent blank
 * (curDC_max = I_DC_MAX * A2BIT_CONV in bldc.c) and must stay >= I_MOT_MAX+2.
 * Hardware ceiling ~40 A (shunt/ADC, A2BIT_CONV=50). Real limit is thermal —
 * watch motor/board temp; prefer short bursts at this level. */
#define I_MOT_MAX                20             // [A] Maximum single motor (phase) current limit
#define I_DC_MAX                 22             // [A] Maximum stage2 DC Link current (>= I_MOT_MAX + 2A)
#define N_MOT_MAX                300            // [rpm] Maximum motor speed limit

/* Encoder-based speed ceiling for torque mode.  The generated controller's
 * legacy speed estimator depends on Hall transitions, which are static in the
 * MT6701 build.  A handwritten PI limiter therefore caps the requested torque
 * from the measured encoder RPM.  KP is command counts per RPM; KI is command
 * counts per RPM per second.  These conservative initial gains are exposed
 * here so bench traces can tune them without touching the controller logic. */
#define ENCODER_SPEED_LIMIT_ENABLE
#define SPEED_LIMIT_KP_CMD_PER_RPM  2048
#define SPEED_LIMIT_KI_CMD_PER_RPM_S 512

/* Sign between the final FOC torque request (after invert_x/y) and the
 * direction-normalized MT6701 speed.  The generated Park/q-axis convention
 * used by this target is opposite: negative torque produces positive encoder
 * RPM.  Keep this per-axis so a different phase mapping can be accommodated
 * without weakening genuine braking/reversing torque. */
#define SPEED_LIMIT_TORQUE_RPM_SIGN_X  (-1)
#define SPEED_LIMIT_TORQUE_RPM_SIGN_Y  (-1)

#ifndef BRING_UP_MODE
#define DC_LINK_WATCHDOG_ENABLE               //Disables the motor without warning incase of under or overvoltage, disable if using hoverboard as vehicle
#endif
#define FIELD_WEAK_ENA           0 
//#define RC_PWM_RIGHT           0         // Use RC PWM as input on the RIGHT cable. (duty cycle mapped to 0 to -1000, 0, 1000) Number indicates priority for dual-input. Disable DEBUG_SERIAL_USART3!
//#define HW_PWM                 0         // [DISABLED for TWO_AXIS+ENCODER_Y — PB5 conflict] Use hw pwm pin PB5 on left side
//#define CONTROL_ADC            1         // use ADC as input pn pins PA2 and PA3, cant be used with extbrk on PA2/PA3
//#define SW_PWM_RIGHT           0         // Use PWM input capture on PB10 and PB11 (duty cycle mapped to 0 to -16000, 0, 16000)
//#define SW_PWM_LEFT            0         // Use PWM input capture on PA2 and PA3 (duty cycle mapped to 0 to -16000, 0, 16000)   (cant be use with extbrk on PA2/PA3)
//#define CONTROL_PPM_LEFT       0         // use PPM-Sum as input on the LEFT cable. Number indicates priority for dual-input. Disable DEBUG_SERIAL_USART2!
//#define PPM_NUM_CHANNELS       1         // total number of PPM channels to receive, even if they are not used.
//#define CONTROL_SERIAL_USART3  0         //  disable if right uart port is used for sw pwm input capture
//#define FEEDBACK_SERIAL_USART3           //  binary telemetry — disabled while DEBUG_SERIAL_USART3 is active
#define DEBUG_SERIAL_USART3                //  ASCII printf via USART3 partial remap: TX=PC10, RX=PC11 (see setup.c)
  #define PRI_INPUT1             0, -32767, 0, 32767,   0   //left motor change depending on input type (may be -32767, 0, 32767)
  #define PRI_INPUT2             2, -32767, 0, 32767,   0   //right motor change depending on input type (may be -32767, 0, 32767)

  #undef RATE
  #undef FILTER 
   #define RATE                   32767     //leave to max rate 32767 if you want instant response (may be needed if you need slower response)                 
  #define FILTER                 65535      //leave to max filter 65535 if you want instant response (may be needed if input is noisy)
  #define INVERT_R_DIRECTION
  //#define INVERT_L_DIRECTION
  /* DEBUG_SERIAL_USART3 is enabled higher up in this variant block */
#endif
/* ===================== Setting up Encoder ===================== */
#ifdef ENCODER_CPR
#define FRAC_CPR ((uint32_t)((1ULL << 32) / (ENCODER_CPR)))
#endif
/* ===================== Finalize PI gains after all variant overrides ===================== */
#ifndef CFG_VBUS_V
#define CFG_VBUS_V                ((float)(BAT_CELLS) * 4.0f)
#endif

#ifdef GD32F103Rx
  #define Vd_max_margin         1627.0f
#else
  #define Vd_max_margin         880.0f
#endif

/* Ensure motor parameters exist even when CFG_USE_BW_PI_CALC is disabled */
#ifndef CFG_MOTOR_R_OHM
  #define CFG_MOTOR_R_OHM          0.3f
#endif
#ifndef CFG_MOTOR_L_H
  #define CFG_MOTOR_L_H            0.0004f
#endif

#ifdef CFG_USE_BW_PI_CALC
#define CFG_CURR_FILT_TARGET_MULT  3U  // [-] EXACT target REAL multiplier for current filter cutoff frequency. Recommended: 3+.
#define CFG_TARGET_BANDWIDTH_HZ_INT ((int)(CFG_TARGET_BANDWIDTH_HZ + 0.5f)) // [Hz] integer mirror derived from bandwidth
#define CFG_PI_CONST_2PI          6.28318530717958647692f
#define CFG_PI_CONST              3.14159265f
#define CFG_TS                     (1.0f / (float)PWM_FREQ)
/* Compile-time upper-bound check: Nyquist Limit
 * Ensure the requested REAL cutoff frequency does not exceed half the PWM frequency.
 */
_Static_assert((CFG_CURR_FILT_TARGET_MULT * CFG_TARGET_BANDWIDTH_HZ_INT) < (PWM_FREQ / 2),
               "BUILD ERROR: Requested Real Filter Cutoff exceeds the Nyquist Limit (PWM_FREQ / 2). Lower the multiplier or bandwidth.");
/* 
 * --- Exact Pre-Warping Calculation ---
 * Calculates the exact internal multiplier required to compensate for digital frequency warping.
 * Uses Taylor approximation: x = R + (R^2*Z)/2 + (R^3*Z^2)/6 + (R^4*Z^3)/24 + (R^5*Z^4)/120
 * Where R is the desired Real Ratio and Z = 2*pi*BW*Ts.
 */
#define CFG_WARP_Z                 (CFG_PI_CONST_2PI * CFG_TARGET_BANDWIDTH_HZ * CFG_TS)
#define CFG_FILT_R                 ((float)CFG_CURR_FILT_TARGET_MULT)
#define CFG_WARPED_MULT            (CFG_FILT_R + \
                                   (((CFG_FILT_R * CFG_FILT_R) * CFG_WARP_Z) / 2.0f) + \
                                   (((CFG_FILT_R * CFG_FILT_R * CFG_FILT_R) * (CFG_WARP_Z * CFG_WARP_Z)) / 6.0f) + \
                                   (((CFG_FILT_R * CFG_FILT_R * CFG_FILT_R * CFG_FILT_R) * (CFG_WARP_Z * CFG_WARP_Z * CFG_WARP_Z)) / 24.0f) + \
                                   (((CFG_FILT_R * CFG_FILT_R * CFG_FILT_R * CFG_FILT_R * CFG_FILT_R) * (CFG_WARP_Z * CFG_WARP_Z * CFG_WARP_Z * CFG_WARP_Z)) / 120.0f))
  #undef QP
  #undef QI
  #undef DP
  #undef DI
  #undef CFG_CURR_FILT
/*
 * Auto formula:
 *   VBUS = BAT_CELLS * 4.2
 *   QP   = 2*pi*Bandwidth*L / VBUS
 *   QI   = QP * R/L
 */
  #define QP            ((CFG_PI_CONST_2PI * CFG_TARGET_BANDWIDTH_HZ * CFG_MOTOR_L_H) * (Vd_max_margin/(CFG_VBUS_V*50))) // [-] P gain
  #define QI            (QP * (CFG_MOTOR_R_OHM / CFG_MOTOR_L_H))                                     // [-] I gain
  #define DP            QP                                                                             // [-] P gain
  #define DI            QI                                                                             // [-] I gain
  /* Current measurement low-pass filter coefficient: fixdt(0,16,16) */
  #define CFG_CURR_FILT                (CFG_TS / (CFG_TS + (1.0f / (CFG_PI_CONST_2PI * (CFG_TARGET_BANDWIDTH_HZ * CFG_WARPED_MULT)))))
  #define CFG_CF_CURR_FILT             FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(CFG_CURR_FILT, 16))
#endif

/* Finalize current filter coefficient after all variant overrides */
#ifndef CFG_CURR_FILT
  #define CFG_CURR_FILT                0.12f
#endif
#ifndef CFG_CF_CURR_FILT
  #define CFG_CF_CURR_FILT             FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(CFG_CURR_FILT, 16))
#endif

/* Final integrator scaling after all QI/DI overrides */
#define QaI              (float)(QI/(PWM_FREQ))      //Integrator scaling//
#define DaI              (float)(DI/(PWM_FREQ))      //Integrator scaling//

/* Gain values pre-scaled for fixed-point model parameters (after variant overrides) */
#define CFG_CF_IQKP                    FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(QP, 10))
#define CFG_CF_IDKP                    FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(DP, 10))
#define CFG_CF_IQKI                    FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(QaI, 16))
#define CFG_CF_IDKI                    FIXDT_CLAMP_U16(FIXDT_FROM_FLOAT(DaI, 16))

/* ===================== Setting up FeedForward Gain( not used anymore) ===================== */
#ifdef FeedForward
#define FeedForwardEnable     1
#define FF_GAIN_REAL                 (((((CFG_MOTOR_R_OHM) / (float)A2BIT_CONV) * ((Vd_max_margin * 2.0f) / CFG_VBUS_V)))) * 0.25f
#define FF_GAIN                      FIXDT_CLAMP_S16(FIXDT_FROM_FLOAT(FF_GAIN_REAL, 12))
#else
#define FeedForwardEnable     0  
#endif


// ########################### UART SETIINGS ############################
#if defined(FEEDBACK_SERIAL_USART2) || defined(CONTROL_SERIAL_USART2) || defined(DEBUG_SERIAL_USART2) || defined(SIDEBOARD_SERIAL_USART2) || \
    defined(FEEDBACK_SERIAL_USART3) || defined(CONTROL_SERIAL_USART3) || defined(DEBUG_SERIAL_USART3) || defined(SIDEBOARD_SERIAL_USART3)
  #define SERIAL_START_FRAME      0xABCD                  // [-] Start frame definition for serial commands
  #define SERIAL_BUFFER_SIZE      64                      // [bytes] Size of Serial Rx buffer. Make sure it is always larger than the structure size
  #define SERIAL_TIMEOUT          160                     // [-] Serial timeout duration for the received data. 160 ~= 0.8 sec. Calculation: 0.8 sec / 0.005 sec
#endif
#if defined(FEEDBACK_SERIAL_USART2) || defined(CONTROL_SERIAL_USART2) || defined(DEBUG_SERIAL_USART2) || defined(SIDEBOARD_SERIAL_USART2)
  #ifndef USART2_BAUD
    #define USART2_BAUD           115200                  // UART2 baud rate (long wired cable)
  #endif
  #define USART2_WORDLENGTH       UART_WORDLENGTH_8B      // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
#endif
#if defined(FEEDBACK_SERIAL_USART3) || defined(CONTROL_SERIAL_USART3) || defined(DEBUG_SERIAL_USART3) || defined(SIDEBOARD_SERIAL_USART3)
  #ifndef USART3_BAUD
    #define USART3_BAUD           115200                  // UART3 baud rate (short wired cable)
  #endif
  #define USART3_WORDLENGTH       UART_WORDLENGTH_8B      // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
#endif
// ########################### UART SETIINGS ############################



// ############################### APPLY DEFAULT SETTINGS ###############################
#ifndef RATE
  #define RATE DEFAULT_RATE
#endif
#ifndef FILTER
  #define FILTER DEFAULT_FILTER
#endif
#ifndef SPEED_COEFFICIENT
  #define SPEED_COEFFICIENT DEFAULT_SPEED_COEFFICIENT
#endif
#ifndef STEER_COEFFICIENT
  #define STEER_COEFFICIENT DEFAULT_STEER_COEFFICIENT
#endif
#if defined(PRI_INPUT1) && defined(PRI_INPUT2) && defined(AUX_INPUT1) && defined(AUX_INPUT2)
  #define INPUTS_NR               2
#else
  #define INPUTS_NR               1
#endif

#ifdef DC_LINK_WATCHDOG_ENABLE
  #define DC_LINK_OVERVOLTAGE_HIGH_X100   (BAT_CALIB_REAL_VOLTAGE + 300)  // e.g. +5 V
  //#define DC_LINK_OVERVOLTAGE_LOW_X100    (BAT_CALIB_REAL_VOLTAGE + 100U)  // e.g. +1 V

  #define DC_LINK_ADC_COUNTS_FROM_X100(vx100) \
    (((vx100) * BAT_CALIB_ADC + (BAT_CALIB_REAL_VOLTAGE / 2)) / BAT_CALIB_REAL_VOLTAGE)

  #define DC_LINK_OVERVOLTAGE_HIGH_COUNTS  DC_LINK_ADC_COUNTS_FROM_X100(DC_LINK_OVERVOLTAGE_HIGH_X100)
  //#define DC_LINK_OVERVOLTAGE_LOW_COUNTS   DC_LINK_ADC_COUNTS_FROM_X100(DC_LINK_OVERVOLTAGE_LOW_X100)

  #if DC_LINK_OVERVOLTAGE_HIGH_COUNTS > 4095
    #error "DC_LINK_OVERVOLTAGE_HIGH_X100 maps beyond 12-bit ADC range"
  #endif

#ifdef GD32F103Rx
  #define mcu_model  1
  #else
  #define mcu_model  0
#endif  
//#if DC_LINK_OVERVOLTAGE_LOW_COUNTS >= DC_LINK_OVERVOLTAGE_HIGH_COUNTS
 //   #error "DC_LINK_OVERVOLTAGE_LOW_X100 must map below the overvoltage high threshold"
 // #endif
#endif
#ifdef ANALOG_BUTTON
  #define POWER_BUTTON_ADC_FULL_SCALE        4096U   // 12-bit ADC
  #define POWER_BUTTON_ADC_REFERENCE_MV      3300U   // ADC reference voltage in millivolts
  #define POWER_BUTTON_DIVIDER_RATIO_X100    1800U   // Resistor divider scaling (e.g. 18.0 => 33V -> 1.83V)
  #define POWER_BUTTON_THRESHOLD_MV          18000U  // Trip point for treating the switch as pressed
  #define POWER_BUTTON_RELEASE_MARGIN_MV     2000U   // Hysteresis below the trip point before we call it released

  #if POWER_BUTTON_RELEASE_MARGIN_MV >= POWER_BUTTON_THRESHOLD_MV
    #error "POWER_BUTTON_RELEASE_MARGIN_MV must be smaller than POWER_BUTTON_THRESHOLD_MV"
  #endif

  #define POWER_BUTTON_ADC_DENOM          ((POWER_BUTTON_ADC_REFERENCE_MV) * (POWER_BUTTON_DIVIDER_RATIO_X100))
  #define POWER_BUTTON_ADC_COUNTS_FROM_MV(mv) \
    (((((mv) * POWER_BUTTON_ADC_FULL_SCALE) * 100ULL) + POWER_BUTTON_ADC_DENOM / 2ULL) / POWER_BUTTON_ADC_DENOM)

  #define ANALOG_BUTTON_PRESSED_MIN    POWER_BUTTON_ADC_COUNTS_FROM_MV(POWER_BUTTON_THRESHOLD_MV)
  #define ANALOG_BUTTON_RELEASE_MAX    POWER_BUTTON_ADC_COUNTS_FROM_MV((POWER_BUTTON_THRESHOLD_MV) - (POWER_BUTTON_RELEASE_MARGIN_MV))
#endif

// ############################### EMERGENCY STOP INPUT ###############################
// #define ESTOP_ENABLE                 // Enable discrete e-stop input on PA3 (shared with EXTBRK_USE_CH4). Comment out to disable.
// #define ESTOP_BUTTON_NO              // Normally Open (active low). Press once to latch until the next press.
// #define ESTOP_BUTTON_NC              // Normally Closed (active high). Do not define together with ESTOP_BUTTON_NO.
// #define ESTOP_REQUIRE_HOLD           // Require the button to stay pressed for the estop to remain active.
#ifdef ESTOP_ENABLE
  #if defined(ESTOP_BUTTON_NO) && defined(ESTOP_BUTTON_NC)
    #error "Define only one of ESTOP_BUTTON_NO or ESTOP_BUTTON_NC"
  #endif
  #if !defined(ESTOP_BUTTON_NO) && !defined(ESTOP_BUTTON_NC)
    #define ESTOP_BUTTON_NO
  #endif
  #ifndef ESTOP_DEBOUNCE_MS
    #define ESTOP_DEBOUNCE_MS   30U     // Debounce window (ms) for the estop input
  #endif
#endif
// ########################### END OF APPLY DEFAULT SETTING ############################



// ############################### VALIDATE SETTINGS ###############################
#if !defined(TWO_AXIS_VARIANT)
  #error Variant not defined! Only TWO_AXIS_VARIANT is supported — check platformio.ini build_flags.
#endif

#if defined(DEBUG_SERIAL_USART2) && defined(DEBUG_SERIAL_USART3)
  #error DEBUG_SERIAL_USART2 and DEBUG_SERIAL_USART3 not allowed, choose one.
#endif

#if defined(INTBRK_L_EN) && defined(MOTOR_LEFT_ENA)
  #error INTBRK_L_EN and MOTOR_LEFT_ENA cannot be used at the same time. Please disable MOTOR_LEFT_ENA when using INTBRK_L_EN.
#endif
#if defined(INTBRK_L_EN) && defined(EXTBRK_EN)
  #error INTBRK_L_EN and EXTBRK_EN cannot be used at the same time. Please choose one braking method.
#endif

#if defined(ESTOP_ENABLE) && defined(EXTBRK_USE_CH4)
  #error ESTOP_ENABLE and EXTBRK_USE_CH4 conflict on PA3. Choose a different brake channel or disable e-stop.
#endif
// ############################# END OF VALIDATE SETTINGS ############################

#endif // CONFIG_H
