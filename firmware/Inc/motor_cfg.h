#ifndef MOTOR_CFG_H
#define MOTOR_CFG_H

#include <stdint.h>

/*
 * Persistent motor configuration. Loaded at boot from EEPROM emulation
 * (Src/eeprom.c, flash pages 64/96). Missing / corrupt data falls back to
 * defaults defined at the top of Src/motor_cfg.c.
 *
 * Fields marked "runtime" apply immediately when set via serial CFG command;
 * fields marked "core" are only re-read at boot after CFG SAVE.
 */

#define MOTOR_CFG_MAGIC     0xA55Au
#define MOTOR_CFG_VERSION   6u   /* legacy CRC layout retained; extensions follow it */

typedef struct {
  int16_t  i_mot_max;         /* A      runtime  Max single-motor phase current */
  int16_t  i_dc_max;          /* A      runtime  Max DC-link current            */
  int16_t  n_mot_max;         /* rpm    runtime  Max motor speed                */
  uint16_t motor_kt_x1000;    /* Nm/A*1000 runtime Motor torque constant Kt      */

  uint8_t  brk_ramp_ena;      /*        runtime  Voltage-based brake fallback   */
  uint8_t  invert_x;          /*        runtime  Invert axis X torque direction */
  uint8_t  invert_y;          /*        runtime  Invert axis Y torque direction */
  uint8_t  _pad0;

  uint16_t brake_res_x100;    /* Ω*100  runtime  Brake resistor value           */
  uint16_t brk_res_sens_ma;   /* mA     runtime  Activation sensitivity         */
  uint16_t max_regen_ma;      /* mA     runtime  Max regen current              */
  uint16_t brk_ov_start_x100; /* V*100  runtime  Overvoltage ramp start         */
  uint16_t brk_ov_end_x100;   /* V*100  runtime  Overvoltage ramp end           */

  uint16_t enc_cpr;           /* counts core     Encoder CPR                    */
  uint16_t bat_nominal_x100;  /* V*100  core     Nominal battery voltage        */
  uint16_t target_bw_hz;      /* Hz     core     FOC target closed-loop BW      */

  uint16_t align_i_x_x100;    /* A*100  runtime  Encoder-X alignment current    */
  uint16_t align_i_y_x100;    /* A*100  runtime  Encoder-Y alignment current    */
  /* Number of electrical cycles the alignment sweep covers. Default 2 (git-
   * original). Higher = wider sweep, better direction resolution, more time
   * at high-power hold for the offset capture to settle. MOVE_MS scales
   * proportionally so mechanical speed stays constant. */
  uint16_t align_turns_elec;  /*        runtime  1..10 (starts new sweep only)  */

  /* Stored alignment result — set by "CFG SAVE_ALIGN" after a successful
   * runtime alignment. When align_stored=1, boot skips auto-align and applies
   * these directly (encoder_[xy].ali is pre-set to true). */
  uint8_t  align_stored;      /*        boot     0 = re-align at boot, 1 = use saved */
  uint8_t  align_x_dir;       /*        boot     encoder_x.direction (0/1)      */
  uint8_t  align_y_dir;       /*        boot     encoder_y.direction (0/1)      */
  uint8_t  _pad2;
  int16_t  align_x_offset;    /* counts boot     encoder_x.offset (electrical)  */
  int16_t  align_y_offset;    /* counts boot     encoder_y.offset (electrical)  */
  /* cnt_offset is the MECHANICAL zero anchor — subtracted from the MT6701's
   * absolute reading to give a rotor-aligned count. Alignment writes this to
   * encoder_[xy].cnt_offset in the last state. Missing it means the FOC
   * computes a wrong electrical angle → motor stalls or runs badly. */
  uint16_t align_x_cnt_offset;/* counts boot     encoder_x.cnt_offset            */
  uint16_t align_y_cnt_offset;/* counts boot     encoder_y.cnt_offset            */
} motor_cfg_t;

extern volatile motor_cfg_t motor_cfg;

/* Initialize EEPROM store and load config into RAM. Falls back to defaults
 * on magic/CRC mismatch. Call once early in boot after HAL_Init/clock config.
 * Returns 1 if valid config was loaded, 0 if defaults were applied. */
uint8_t motor_cfg_init(void);

/* Persist the current RAM config to EEPROM. Returns 0 on success. */
uint8_t motor_cfg_save(void);

/* Reset RAM copy to compile-time defaults. Not persisted until save(). */
void    motor_cfg_reset_defaults(void);

/* Set a field by name (case-insensitive). Returns 0 ok, -1 unknown name,
 * -2 out of range. Value string parsed with strtol/strtof depending on field. */
int8_t  motor_cfg_set(const char *name, const char *value);

/* Emit the whole config as human-readable "KEY: VALUE" lines via printf.
 * Used by the "?CFG" serial command. */
void    motor_cfg_dump(void);

/* Re-push runtime-effective fields into the live control state (rtP_Left/Right
 * limits, etc.). Call after motor_cfg_set() or motor_cfg_reset_defaults() so
 * changes take effect without a reboot. Core-only fields (enc_cpr, target_bw_hz,
 * bat_nominal) are read at boot and skipped here. */
void    motor_cfg_apply_runtime(void);

/* Snapshot the current encoder alignment result (encoder_x/y.offset/direction)
 * into motor_cfg and persist to EEPROM with align_stored=1. Returns 0 on
 * success, 1 if some axis wasn't aligned yet, 2 on flash write error. */
uint8_t motor_cfg_capture_alignment(void);

/* If align_stored=1, push the saved offset/direction into encoder_[xy] and
 * mark them aligned so main.c skips Encoder_[XY]_Align_Start. Safe to call
 * even if only one axis is compiled in — the other one just becomes a no-op.
 * Per-axis variants exist so Encoder_[XY]_Init can reapply the stored state
 * right after its own reset — otherwise the boot-time apply gets clobbered. */
void    motor_cfg_apply_stored_alignment(void);
void    motor_cfg_apply_stored_alignment_x(void);
void    motor_cfg_apply_stored_alignment_y(void);

#endif /* MOTOR_CFG_H */
