#include "motor_cfg.h"
#include "config.h"
#include "eeprom.h"
#include "BLDC_controller.h"
#include "rtwtypes.h"
#include "util.h"          /* Encoder struct + globals encoder_x / encoder_y */

extern P rtP_Left;
extern P rtP_Right;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Local case-insensitive strcmp — <strings.h> isn't standard C. */
static int ci_strcmp(const char *a, const char *b)
{
  while (*a && *b) {
    int ca = tolower((unsigned char)*a);
    int cb = tolower((unsigned char)*b);
    if (ca != cb) return ca - cb;
    a++; b++;
  }
  return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* ==== Compile-time defaults ==================================================
 * The reset path pulls these values back into the RAM copy. Keep them in
 * sync with the "sane starting point" values in config.h.
 */
#define DEFAULT_I_MOT_MAX          20      /* A */
#define DEFAULT_I_DC_MAX           22      /* A */
#define DEFAULT_N_MOT_MAX          300     /* rpm */
#define DEFAULT_MOTOR_KT_X1000     1000    /* 1.000 N.m/A — replace with motor data */
#define DEFAULT_BRK_RAMP_ENA       1
#define DEFAULT_INVERT_X           0
#define DEFAULT_INVERT_Y           0
#define DEFAULT_BRAKE_RES_X100     300     /* 3.00 Ω */
#define DEFAULT_BRK_RES_SENS_MA    40      /* mA — matches old `40 / 20` = 2 in 20 mA steps */
#define DEFAULT_MAX_REGEN_MA       0
#define DEFAULT_BRK_OV_START_X100  4100    /* V*100 (4.10 V/cel × 10 cel) */
#define DEFAULT_BRK_OV_END_X100    4300    /* V*100 (4.30 V/cel × 10 cel) */
#define DEFAULT_ENC_CPR            4096
#define DEFAULT_BAT_NOMINAL_X100   3700    /* V*100 = 37.00V (10s Li-ion nominal, 3.7 V/cel) */
#define DEFAULT_TARGET_BW_HZ       300     /* Hz */
/* Alignment current: git-original ALIGNMENT_X/Y_POWER = 3000 fixdt(1,16,4)
 * = 3000/(A2BIT_CONV*16) = 3.75 A. Stored as A*100 so user sets an absolute
 * current independent of i_mot_max (avoids the "align current shrinks when I
 * lower i_mot_max" problem). Cap by i_mot_max still applies at FOC saturation. */
#define DEFAULT_ALIGN_I_X_X100     375     /* 3.75 A */
#define DEFAULT_ALIGN_I_Y_X100     375     /* 3.75 A */
#define DEFAULT_ALIGN_TURNS_ELEC   2       /* git-original sweep (2 electrical cycles) */
#define DEFAULT_ALIGN_STORED       0       /* boot: run auto-align (no saved value) */
#define DEFAULT_ALIGN_X_DIR        0
#define DEFAULT_ALIGN_Y_DIR        0
#define DEFAULT_ALIGN_X_OFFSET     0
#define DEFAULT_ALIGN_Y_OFFSET     0
#define DEFAULT_ALIGN_X_CNT_OFFSET 0
#define DEFAULT_ALIGN_Y_CNT_OFFSET 0

/* ==== EEPROM slot layout =====================================================
 * Slots 1000..1018 are the legacy input-config table (still declared in
 * util.c). Motor-config slots start at 2000 so they never collide with legacy
 * data even if someone flashes an old firmware over a new EEPROM.
 *
 * Slot 0 stores the guard (magic << 0) | (version << 8) so a single read
 * validates both. SLOT_CRC and everything before it retain the v6 layout.
 * Optional backward-compatible fields are appended after SLOT_CRC, allowing
 * boards already carrying a valid v6 configuration/alignment to keep it.
 */
enum {
  SLOT_GUARD = 0,
  SLOT_I_MOT_MAX,
  SLOT_I_DC_MAX,
  SLOT_N_MOT_MAX,
  SLOT_FLAGS,           /* brk_ramp_ena | invert_x<<1 | invert_y<<2 */
  SLOT_BRAKE_RES,
  SLOT_BRK_RES_SENS,
  SLOT_MAX_REGEN,
  SLOT_BRK_OV_START,
  SLOT_BRK_OV_END,
  SLOT_ENC_CPR,
  SLOT_BAT_NOMINAL,
  SLOT_TARGET_BW,
  SLOT_ALIGN_I_X,
  SLOT_ALIGN_I_Y,
  SLOT_ALIGN_STATE,       /* bit0 align_stored, bit1 align_x_dir, bit2 align_y_dir */
  SLOT_ALIGN_X_OFFSET,    /* int16 in a u16 slot */
  SLOT_ALIGN_Y_OFFSET,    /* int16 in a u16 slot */
  SLOT_ALIGN_X_CNT_OFFSET,/* uint16 mechanical zero anchor 0..CPR-1 */
  SLOT_ALIGN_Y_CNT_OFFSET,
  SLOT_ALIGN_TURNS_ELEC,
  SLOT_CRC,
  SLOT_MOTOR_KT,
  SLOT_COUNT_
};
#define MOTOR_CFG_SLOT_BASE  2000u

volatile motor_cfg_t motor_cfg;

/* ==== CRC-16 (CCITT/False, poly 0x1021, seed 0xFFFF) ========================= */
static uint16_t crc16_update(uint16_t crc, uint16_t v)
{
  crc ^= (uint16_t)((v >> 8) << 8);
  for (int i = 0; i < 8; i++) crc = (crc & 0x8000u) ? (crc << 1) ^ 0x1021u : (crc << 1);
  crc ^= (uint16_t)((v & 0xFFu) << 8);
  for (int i = 0; i < 8; i++) crc = (crc & 0x8000u) ? (crc << 1) ^ 0x1021u : (crc << 1);
  return crc;
}

/* Pack the flags nibble (bit 0 = brk_ramp_ena, 1 = invert_x, 2 = invert_y). */
static uint16_t pack_flags(void)
{
  return (uint16_t)((motor_cfg.brk_ramp_ena & 1u) |
                    ((motor_cfg.invert_x   & 1u) << 1) |
                    ((motor_cfg.invert_y   & 1u) << 2));
}
static void unpack_flags(uint16_t f)
{
  motor_cfg.brk_ramp_ena = (uint8_t)( f       & 1u);
  motor_cfg.invert_x     = (uint8_t)((f >> 1) & 1u);
  motor_cfg.invert_y     = (uint8_t)((f >> 2) & 1u);
}

/* Read value that would land in slot i, from the in-RAM struct. */
static uint16_t slot_from_cfg(uint16_t slot)
{
  switch (slot) {
    case SLOT_GUARD:         return (uint16_t)((MOTOR_CFG_MAGIC & 0xFFu) | (MOTOR_CFG_VERSION << 8));
    case SLOT_I_MOT_MAX:     return (uint16_t)motor_cfg.i_mot_max;
    case SLOT_I_DC_MAX:      return (uint16_t)motor_cfg.i_dc_max;
    case SLOT_N_MOT_MAX:     return (uint16_t)motor_cfg.n_mot_max;
    case SLOT_FLAGS:         return pack_flags();
    case SLOT_BRAKE_RES:     return motor_cfg.brake_res_x100;
    case SLOT_BRK_RES_SENS:  return motor_cfg.brk_res_sens_ma;
    case SLOT_MAX_REGEN:     return motor_cfg.max_regen_ma;
    case SLOT_BRK_OV_START:  return motor_cfg.brk_ov_start_x100;
    case SLOT_BRK_OV_END:    return motor_cfg.brk_ov_end_x100;
    case SLOT_ENC_CPR:       return motor_cfg.enc_cpr;
    case SLOT_BAT_NOMINAL:   return motor_cfg.bat_nominal_x100;
    case SLOT_TARGET_BW:     return motor_cfg.target_bw_hz;
    case SLOT_ALIGN_I_X:     return motor_cfg.align_i_x_x100;
    case SLOT_ALIGN_I_Y:     return motor_cfg.align_i_y_x100;
    case SLOT_ALIGN_STATE:   return (uint16_t)((motor_cfg.align_stored & 1u) |
                                    ((motor_cfg.align_x_dir & 1u) << 1) |
                                    ((motor_cfg.align_y_dir & 1u) << 2));
    case SLOT_ALIGN_X_OFFSET:return (uint16_t)motor_cfg.align_x_offset;
    case SLOT_ALIGN_Y_OFFSET:return (uint16_t)motor_cfg.align_y_offset;
    case SLOT_ALIGN_X_CNT_OFFSET:return motor_cfg.align_x_cnt_offset;
    case SLOT_ALIGN_Y_CNT_OFFSET:return motor_cfg.align_y_cnt_offset;
    case SLOT_ALIGN_TURNS_ELEC:return motor_cfg.align_turns_elec;
    case SLOT_MOTOR_KT:      return motor_cfg.motor_kt_x1000;
    default:                 return 0;
  }
}
static void slot_to_cfg(uint16_t slot, uint16_t v)
{
  switch (slot) {
    case SLOT_I_MOT_MAX:     motor_cfg.i_mot_max          = (int16_t)v; break;
    case SLOT_I_DC_MAX:      motor_cfg.i_dc_max           = (int16_t)v; break;
    case SLOT_N_MOT_MAX:     motor_cfg.n_mot_max          = (int16_t)v; break;
    case SLOT_FLAGS:         unpack_flags(v);                           break;
    case SLOT_BRAKE_RES:     motor_cfg.brake_res_x100     = v;          break;
    case SLOT_BRK_RES_SENS:  motor_cfg.brk_res_sens_ma    = v;          break;
    case SLOT_MAX_REGEN:     motor_cfg.max_regen_ma       = v;          break;
    case SLOT_BRK_OV_START:  motor_cfg.brk_ov_start_x100  = v;          break;
    case SLOT_BRK_OV_END:    motor_cfg.brk_ov_end_x100    = v;          break;
    case SLOT_ENC_CPR:       motor_cfg.enc_cpr            = v;          break;
    case SLOT_BAT_NOMINAL:   motor_cfg.bat_nominal_x100   = v;          break;
    case SLOT_TARGET_BW:     motor_cfg.target_bw_hz       = v;          break;
    case SLOT_ALIGN_I_X:     motor_cfg.align_i_x_x100     = v;          break;
    case SLOT_ALIGN_I_Y:     motor_cfg.align_i_y_x100     = v;          break;
    case SLOT_ALIGN_STATE:   motor_cfg.align_stored = (uint8_t)( v       & 1u);
                             motor_cfg.align_x_dir  = (uint8_t)((v >> 1) & 1u);
                             motor_cfg.align_y_dir  = (uint8_t)((v >> 2) & 1u);
                             break;
    case SLOT_ALIGN_X_OFFSET:motor_cfg.align_x_offset     = (int16_t)v;  break;
    case SLOT_ALIGN_Y_OFFSET:motor_cfg.align_y_offset     = (int16_t)v;  break;
    case SLOT_ALIGN_X_CNT_OFFSET:motor_cfg.align_x_cnt_offset = v;       break;
    case SLOT_ALIGN_Y_CNT_OFFSET:motor_cfg.align_y_cnt_offset = v;       break;
    case SLOT_ALIGN_TURNS_ELEC:motor_cfg.align_turns_elec = v;           break;
    case SLOT_MOTOR_KT:      motor_cfg.motor_kt_x1000     = v;           break;
    default: break;
  }
}

void motor_cfg_apply_runtime(void)
{
  /* Match Input_Lim_Init (util.c): both fields are fixdt(1,16,4) — the raw
   * value stored is the human unit × 16. i_max additionally scales by
   * A2BIT_CONV so 1 A = A2BIT_CONV ADC counts.
   *   i_max = amps  × A2BIT_CONV × 16
   *   n_max = rpm   × 16
   * Skipping the << 4 makes the actual limit 16× smaller — with i_mot_max=20
   * and A2BIT_CONV=50, motors were being capped at ~1.25 A instead of 20 A. */
  rtP_Left.i_max = rtP_Right.i_max = (int16_t)((int32_t)motor_cfg.i_mot_max * A2BIT_CONV * 16);
  rtP_Left.n_max = rtP_Right.n_max = (int16_t)((int32_t)motor_cfg.n_mot_max << 4);
  /* i_dc_max / invert_x / invert_y / brake_* are read live from motor_cfg
   * on every ISR iteration — nothing else to push here. */
}

void motor_cfg_reset_defaults(void)
{
  motor_cfg.i_mot_max          = DEFAULT_I_MOT_MAX;
  motor_cfg.i_dc_max           = DEFAULT_I_DC_MAX;
  motor_cfg.n_mot_max          = DEFAULT_N_MOT_MAX;
  motor_cfg.motor_kt_x1000     = DEFAULT_MOTOR_KT_X1000;
  motor_cfg.brk_ramp_ena       = DEFAULT_BRK_RAMP_ENA;
  motor_cfg.invert_x           = DEFAULT_INVERT_X;
  motor_cfg.invert_y           = DEFAULT_INVERT_Y;
  motor_cfg._pad0              = 0;
  motor_cfg.brake_res_x100     = DEFAULT_BRAKE_RES_X100;
  motor_cfg.brk_res_sens_ma    = DEFAULT_BRK_RES_SENS_MA;
  motor_cfg.max_regen_ma       = DEFAULT_MAX_REGEN_MA;
  motor_cfg.brk_ov_start_x100  = DEFAULT_BRK_OV_START_X100;
  motor_cfg.brk_ov_end_x100    = DEFAULT_BRK_OV_END_X100;
  motor_cfg.enc_cpr            = DEFAULT_ENC_CPR;
  motor_cfg.bat_nominal_x100   = DEFAULT_BAT_NOMINAL_X100;
  motor_cfg.target_bw_hz       = DEFAULT_TARGET_BW_HZ;
  motor_cfg.align_i_x_x100     = DEFAULT_ALIGN_I_X_X100;
  motor_cfg.align_i_y_x100     = DEFAULT_ALIGN_I_Y_X100;
  motor_cfg.align_stored       = DEFAULT_ALIGN_STORED;
  motor_cfg.align_x_dir        = DEFAULT_ALIGN_X_DIR;
  motor_cfg.align_y_dir        = DEFAULT_ALIGN_Y_DIR;
  motor_cfg._pad2              = 0;
  motor_cfg.align_x_offset     = DEFAULT_ALIGN_X_OFFSET;
  motor_cfg.align_y_offset     = DEFAULT_ALIGN_Y_OFFSET;
  motor_cfg.align_x_cnt_offset = DEFAULT_ALIGN_X_CNT_OFFSET;
  motor_cfg.align_y_cnt_offset = DEFAULT_ALIGN_Y_CNT_OFFSET;
  motor_cfg.align_turns_elec   = DEFAULT_ALIGN_TURNS_ELEC;
}

uint8_t motor_cfg_init(void)
{
  /* EE_Init() is already called earlier by Input_Init(); do not call it again. */
  uint16_t guard = 0;
  if (EE_ReadVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + SLOT_GUARD), &guard) != 0 ||
      guard != (uint16_t)((MOTOR_CFG_MAGIC & 0xFFu) | (MOTOR_CFG_VERSION << 8))) {
    motor_cfg_reset_defaults();
    return 0;
  }

  /* Load slots into RAM, computing CRC over what we read (excluding the CRC slot). */
  uint16_t crc = 0xFFFFu;
  crc = crc16_update(crc, guard);
  for (uint16_t s = SLOT_I_MOT_MAX; s < SLOT_CRC; s++) {
    uint16_t v = 0;
    if (EE_ReadVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + s), &v) != 0) {
      motor_cfg_reset_defaults();
      return 0;
    }
    slot_to_cfg(s, v);
    crc = crc16_update(crc, v);
  }

  uint16_t stored_crc = 0;
  if (EE_ReadVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + SLOT_CRC), &stored_crc) != 0 ||
      stored_crc != crc) {
    motor_cfg_reset_defaults();
    return 0;
  }

  /* Kt was appended after the v6 CRC. Old saved configurations simply do not
   * have this virtual address yet and receive the neutral 1.000 N.m/A default. */
  motor_cfg.motor_kt_x1000 = DEFAULT_MOTOR_KT_X1000;
  {
    uint16_t motor_kt = 0;
    if (EE_ReadVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + SLOT_MOTOR_KT), &motor_kt) == 0 &&
        motor_kt >= 1u) {
      motor_cfg.motor_kt_x1000 = motor_kt;
    }
  }
  return 1;
}

uint8_t motor_cfg_save(void)
{
  HAL_FLASH_Unlock();
  uint16_t crc = 0xFFFFu;
  uint16_t rc  = 0;
  for (uint16_t s = SLOT_GUARD; s < SLOT_CRC; s++) {
    uint16_t v = slot_from_cfg(s);
    rc |= EE_WriteVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + s), v);
    crc = crc16_update(crc, v);
  }
  rc |= EE_WriteVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + SLOT_CRC), crc);
  rc |= EE_WriteVariable((uint16_t)(MOTOR_CFG_SLOT_BASE + SLOT_MOTOR_KT),
                         motor_cfg.motor_kt_x1000);
  HAL_FLASH_Lock();
  return rc ? 1u : 0u;
}

/* ==== Serial helpers ========================================================= */
/* Field table for set/dump. `is_bool` means clamp to 0/1. `scale` prints x/scale
 * as float; parse divides input by scale. For plain int fields scale = 0. */
typedef struct {
  const char *name;
  uint16_t    slot;
  uint16_t    min;
  uint16_t    max;
  uint8_t     is_signed;   /* value stored as int16 (still fits in uint16 slot) */
  uint8_t     is_bool;
  uint16_t    scale;       /* 0 = integer, otherwise printed/parsed as float / scale */
} cfg_field_t;

static const cfg_field_t FIELDS[] = {
  /* i_mot_max upper bound: i_max is stored as amps*A2BIT_CONV*16 in int16,
   * so max_amps ≤ 32767/(A2BIT_CONV*16) = 40.9 A with the default A2BIT_CONV. */
  { "i_mot_max",     SLOT_I_MOT_MAX,    1,   40,   1, 0, 0   },
  { "i_dc_max",      SLOT_I_DC_MAX,     1,   60,   1, 0, 0   },
  /* n_mot_max stored as rpm*16 in int16 → max_rpm ≤ 32767/16 = 2047. */
  { "n_mot_max",     SLOT_N_MOT_MAX,    1, 2000,   1, 0, 0   },
  { "motor_kt",      SLOT_MOTOR_KT,     1, 65535,  0, 0, 1000 }, /* N.m/A, 3 casas */
  { "brk_ramp_ena",  0xFFFFu,           0,    1,   0, 1, 0   },
  { "invert_x",      0xFFFFu,           0,    1,   0, 1, 0   },
  { "invert_y",      0xFFFFu,           0,    1,   0, 1, 0   },
  { "brake_res",     SLOT_BRAKE_RES,   10, 5000,   0, 0, 100 },  /* Ω, 2 casas */
  { "brk_res_sens",  SLOT_BRK_RES_SENS, 0, 10000,  0, 0, 0   },  /* mA */
  { "max_regen",     SLOT_MAX_REGEN,    0, 10000,  0, 0, 0   },  /* mA */
  { "brk_ov_start",  SLOT_BRK_OV_START, 100, 10000,0, 0, 100 },  /* V, 2 casas */
  { "brk_ov_end",    SLOT_BRK_OV_END,   100, 10000,0, 0, 100 },  /* V, 2 casas */
  { "enc_cpr",       SLOT_ENC_CPR,      1, 65000,  0, 0, 0   },
  { "bat_nominal",   SLOT_BAT_NOMINAL,  100, 8000, 0, 0, 100 },  /* V, 2 casas */
  { "target_bw_hz",  SLOT_TARGET_BW,   10,  2000,  0, 0, 0   },  /* Hz */
  /* Alignment currents: absolute A (independent of i_mot_max). Peak used
   * internally is 2×align_i (state 2 doubles), so max here is ~half the
   * i_mot_max ceiling to keep saturation from clipping the sweep. */
  { "align_i_x",     SLOT_ALIGN_I_X,    10, 2000,  0, 0, 100 },  /* A, 2 casas */
  { "align_i_y",     SLOT_ALIGN_I_Y,    10, 2000,  0, 0, 100 },  /* A, 2 casas */
  { "align_turns_elec", SLOT_ALIGN_TURNS_ELEC, 1, 10, 0, 0, 0 }, /* elec cycles per sweep */
  /* Stored alignment result — set by "CFG SAVE_ALIGN". Users can flip
   * align_stored back to 0 (via CFG align_stored=0 + CFG SAVE) to force a
   * fresh auto-align on the next boot. */
  { "align_stored",  0xFFFFu,           0,    1,   0, 1, 0   },
  { "align_x_offset",SLOT_ALIGN_X_OFFSET, 0, 65535,1, 0, 0   },  /* int16 in u16 slot */
  { "align_x_dir",   0xFFFFu,           0,    1,   0, 1, 0   },
  { "align_x_cnt_offset",SLOT_ALIGN_X_CNT_OFFSET, 0, 65535, 0, 0, 0 },
  { "align_y_offset",SLOT_ALIGN_Y_OFFSET, 0, 65535,1, 0, 0   },
  { "align_y_dir",   0xFFFFu,           0,    1,   0, 1, 0   },
  { "align_y_cnt_offset",SLOT_ALIGN_Y_CNT_OFFSET, 0, 65535, 0, 0, 0 },
};
#define FIELD_COUNT (sizeof(FIELDS) / sizeof(FIELDS[0]))

static uint16_t read_field(const cfg_field_t *f)
{
  if (f->slot != 0xFFFFu) return slot_from_cfg(f->slot);
  /* Flags-only fields */
  if (strcmp(f->name, "brk_ramp_ena") == 0) return motor_cfg.brk_ramp_ena;
  if (strcmp(f->name, "invert_x")     == 0) return motor_cfg.invert_x;
  if (strcmp(f->name, "invert_y")     == 0) return motor_cfg.invert_y;
  if (strcmp(f->name, "align_stored") == 0) return motor_cfg.align_stored;
  if (strcmp(f->name, "align_x_dir")  == 0) return motor_cfg.align_x_dir;
  if (strcmp(f->name, "align_y_dir")  == 0) return motor_cfg.align_y_dir;
  return 0;
}
static void write_field(const cfg_field_t *f, uint16_t v)
{
  if (f->slot != 0xFFFFu) { slot_to_cfg(f->slot, v); return; }
  if (strcmp(f->name, "brk_ramp_ena") == 0) motor_cfg.brk_ramp_ena = (uint8_t)(v & 1u);
  if (strcmp(f->name, "invert_x")     == 0) motor_cfg.invert_x     = (uint8_t)(v & 1u);
  if (strcmp(f->name, "invert_y")     == 0) motor_cfg.invert_y     = (uint8_t)(v & 1u);
  if (strcmp(f->name, "align_stored") == 0) motor_cfg.align_stored = (uint8_t)(v & 1u);
  if (strcmp(f->name, "align_x_dir")  == 0) motor_cfg.align_x_dir  = (uint8_t)(v & 1u);
  if (strcmp(f->name, "align_y_dir")  == 0) motor_cfg.align_y_dir  = (uint8_t)(v & 1u);
}

int8_t motor_cfg_set(const char *name, const char *value)
{
  for (unsigned i = 0; i < FIELD_COUNT; i++) {
    const cfg_field_t *f = &FIELDS[i];
    if (ci_strcmp(f->name, name) != 0) continue;

    long parsed;
    if (f->scale > 0) {
      /* Float input like "24.0" — scale up by f->scale and round. */
      float fv = strtof(value, NULL);
      parsed = (long)(fv * (float)f->scale + (fv >= 0.0f ? 0.5f : -0.5f));
    } else {
      parsed = strtol(value, NULL, 10);
    }
    if (parsed < (long)f->min || parsed > (long)f->max) return -2;
    write_field(f, (uint16_t)parsed);
    return 0;
  }
  return -1;
}

void motor_cfg_dump(void)
{
  printf("CFG {\r\n");
  for (unsigned i = 0; i < FIELD_COUNT; i++) {
    const cfg_field_t *f = &FIELDS[i];
    uint16_t raw = read_field(f);
    if (f->is_bool) {
      printf("  %s: %u\r\n", f->name, (unsigned)(raw & 1u));
    } else if (f->scale > 0) {
      unsigned whole = raw / f->scale;
      unsigned frac  = raw - whole * f->scale;
      if (f->scale == 1000u) {
        printf("  %s: %u.%03u\r\n", f->name, whole, frac);
      } else {
        printf("  %s: %u.%02u\r\n", f->name, whole, frac);
      }
    } else if (f->is_signed) {
      printf("  %s: %d\r\n", f->name, (int)(int16_t)raw);
    } else {
      printf("  %s: %u\r\n", f->name, (unsigned)raw);
    }
  }
  /* Live encoder diagnostic snapshot — read-only, not part of the persisted
   * struct. Lets the dashboard show "current runtime" vs "saved on flash" so
   * the user knows whether the boot align produced non-zero offsets before
   * they click SAVE_ALIGN. */
#if defined(ENCODER_X)
  {
    extern SensorState encoder_x;
    printf("  align_x_offset_live: %d\r\n",     (int)encoder_x.offset);
    printf("  align_x_dir_live: %u\r\n",        (unsigned)(encoder_x.direction ? 1u : 0u));
    printf("  align_x_cnt_offset_live: %ld\r\n",(long)encoder_x.cnt_offset);
    printf("  align_x_ali: %u\r\n",             (unsigned)(encoder_x.ali       ? 1u : 0u));
  }
#endif
#if defined(ENCODER_Y)
  {
    extern SensorState encoder_y;
    printf("  align_y_offset_live: %d\r\n",     (int)encoder_y.offset);
    printf("  align_y_dir_live: %u\r\n",        (unsigned)(encoder_y.direction ? 1u : 0u));
    printf("  align_y_cnt_offset_live: %ld\r\n",(long)encoder_y.cnt_offset);
    printf("  align_y_ali: %u\r\n",             (unsigned)(encoder_y.ali       ? 1u : 0u));
  }
#endif
  printf("}\r\n");
}

/* ==== Stored alignment (encoder offset persistence) ========================
 * Captures whatever encoder_[xy] currently hold — call only after both axes
 * have finished a successful auto-align (encoder_[xy].ali == true).
 * Refuses if either compiled-in axis is still un-aligned. */
uint8_t motor_cfg_capture_alignment(void)
{
#if defined(ENCODER_X)
  extern SensorState encoder_x;
  if (!encoder_x.ali) return 1u;
  motor_cfg.align_x_offset     = (int16_t)encoder_x.offset;
  motor_cfg.align_x_dir        = encoder_x.direction ? 1u : 0u;
  motor_cfg.align_x_cnt_offset = (uint16_t)encoder_x.cnt_offset;
#endif
#if defined(ENCODER_Y)
  extern SensorState encoder_y;
  if (!encoder_y.ali) return 1u;
  motor_cfg.align_y_offset     = (int16_t)encoder_y.offset;
  motor_cfg.align_y_dir        = encoder_y.direction ? 1u : 0u;
  motor_cfg.align_y_cnt_offset = (uint16_t)encoder_y.cnt_offset;
#endif
  motor_cfg.align_stored = 1u;
  return motor_cfg_save() ? 2u : 0u;
}

void motor_cfg_apply_stored_alignment_x(void)
{
#if defined(ENCODER_X)
  if (!motor_cfg.align_stored) return;
  extern SensorState encoder_x;
  encoder_x.offset      = motor_cfg.align_x_offset;
  encoder_x.direction   = motor_cfg.align_x_dir ? 1 : 0;
  encoder_x.cnt_offset  = (int32_t)motor_cfg.align_x_cnt_offset;
  encoder_x.ali         = true;
  encoder_x.align_fault = false;
  rtP_Right.b_diagEna   = DIAG_ENA;
#endif
}
void motor_cfg_apply_stored_alignment_y(void)
{
#if defined(ENCODER_Y)
  if (!motor_cfg.align_stored) return;
  extern SensorState encoder_y;
  encoder_y.offset      = motor_cfg.align_y_offset;
  encoder_y.direction   = motor_cfg.align_y_dir ? 1 : 0;
  encoder_y.cnt_offset  = (int32_t)motor_cfg.align_y_cnt_offset;
  encoder_y.ali         = true;
  encoder_y.align_fault = false;
  rtP_Left.b_diagEna    = DIAG_ENA;
#endif
}
void motor_cfg_apply_stored_alignment(void)
{
  motor_cfg_apply_stored_alignment_x();
  motor_cfg_apply_stored_alignment_y();
}
