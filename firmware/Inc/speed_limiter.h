#ifndef SPEED_LIMITER_H
#define SPEED_LIMITER_H

#include <stdint.h>

typedef struct {
  int64_t  integral_q16;
  uint32_t last_speed_tick;
  int16_t  cap_magnitude;
  int16_t  applied_cmd;
  int16_t  integral_cmd;
  uint8_t  active;
  uint8_t  feedback_fault;
} SpeedLimiterState;

extern SpeedLimiterState speed_limiter_x;
extern SpeedLimiterState speed_limiter_y;

void speed_limiter_reset(SpeedLimiterState *state);

/* Apply an encoder-speed ceiling to a torque-mode command.  requested_cmd is
 * the final signed command after per-axis inversion; rpm_q4 is signed RPM*16.
 * A command opposing measured rotation is passed through so braking is never
 * weakened by the speed ceiling. */
int16_t speed_limiter_apply(SpeedLimiterState *state,
                            int16_t requested_cmd,
                            int16_t rpm_q4,
                            int8_t torque_rpm_sign,
                            uint8_t rpm_valid,
                            uint32_t speed_tick,
                            int16_t max_rpm,
                            uint8_t motor_enabled);

#endif
