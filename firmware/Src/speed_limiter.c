#include "speed_limiter.h"
#include "config.h"
#include <string.h>

SpeedLimiterState speed_limiter_x;
SpeedLimiterState speed_limiter_y;

static int32_t clamp_i32(int32_t value, int32_t lo, int32_t hi)
{
  if (value < lo) return lo;
  if (value > hi) return hi;
  return value;
}

void speed_limiter_reset(SpeedLimiterState *state)
{
  memset(state, 0, sizeof(*state));
}

int16_t speed_limiter_apply(SpeedLimiterState *state,
                            int16_t requested_cmd,
                            int16_t rpm_q4,
                            int8_t torque_rpm_sign,
                            uint8_t rpm_valid,
                            uint32_t speed_tick,
                            int16_t max_rpm,
                            uint8_t motor_enabled)
{
  int32_t request_mag = requested_cmd < 0 ? -(int32_t)requested_cmd
                                          :  (int32_t)requested_cmd;
  int32_t rpm_mag = rpm_q4 < 0 ? -(int32_t)rpm_q4 : (int32_t)rpm_q4;
  int32_t aligned_rpm_q4 = (int32_t)rpm_q4 * (torque_rpm_sign < 0 ? -1 : 1);

  if (!motor_enabled || requested_cmd == 0 || max_rpm <= 0) {
    speed_limiter_reset(state);
    return 0;
  }

  /* Once alignment is complete, stale/invalid speed feedback must fail safe.
   * Continuing torque with a frozen absolute angle is unsafe for FOC anyway. */
  if (!rpm_valid) {
    speed_limiter_reset(state);
    state->feedback_fault = 1;
    return 0;
  }

  /* Preserve full braking/reversing authority.  Encoder direction alignment
   * and FOC q-axis torque polarity are separate conventions, so compare the
   * command against the explicitly polarity-corrected speed. */
  if (rpm_mag > 16 && ((requested_cmd < 0) != (aligned_rpm_q4 < 0))) {
    speed_limiter_reset(state);
    state->cap_magnitude = (int16_t)request_mag;
    state->applied_cmd = requested_cmd;
    return requested_cmd;
  }

  /* Recalculate P/I only for a new encoder sample (~994 Hz), not on every
   * 16 kHz FOC interrupt.  The actual sample interval keeps Ki independent of
   * occasional missed PWM frames. */
  if (speed_tick != state->last_speed_tick) {
    uint32_t dt_ticks = state->last_speed_tick == 0u
                      ? (PWM_FREQ / MT6701_SAMPLE_RATE_HZ)
                      : (uint32_t)(speed_tick - state->last_speed_tick);
    if (dt_ticks == 0u) dt_ticks = 1u;
    if (dt_ticks > (PWM_FREQ / 10u)) dt_ticks = PWM_FREQ / 10u;
    state->last_speed_tick = speed_tick;

    int32_t error_q4 = ((int32_t)max_rpm << 4) - rpm_mag;
    int32_t p_cmd = (error_q4 * SPEED_LIMIT_KP_CMD_PER_RPM) >> 4;
    int32_t i_cmd = (int32_t)(state->integral_q16 >> 16);
    int32_t unsaturated = p_cmd + i_cmd;

    /* Conditional integration is the anti-windup mechanism: do not build
     * integral while the requested torque is already the active ceiling, but
     * always allow error that drives the controller back out of saturation. */
    uint8_t integrate = ((unsaturated > 0 && unsaturated < request_mag) ||
                         (unsaturated >= request_mag && error_q4 < 0) ||
                         (unsaturated <= 0 && error_q4 > 0));
    if (integrate) {
      int64_t delta_i = (int64_t)error_q4 * SPEED_LIMIT_KI_CMD_PER_RPM_S
                      * (int64_t)dt_ticks * 65536LL;
      delta_i /= ((int64_t)16 * PWM_FREQ);
      state->integral_q16 += delta_i;
    }

    int64_t integral_max = (int64_t)request_mag << 16;
    if (state->integral_q16 < 0) state->integral_q16 = 0;
    if (state->integral_q16 > integral_max) state->integral_q16 = integral_max;
    state->integral_cmd = (int16_t)(state->integral_q16 >> 16);

    unsaturated = p_cmd + state->integral_cmd;
    state->cap_magnitude = (int16_t)clamp_i32(unsaturated, 0, request_mag);
  }

  int32_t applied_mag = state->cap_magnitude;
  if (applied_mag > request_mag) applied_mag = request_mag;
  state->active = (applied_mag < request_mag);
  state->feedback_fault = 0;
  state->applied_cmd = requested_cmd < 0 ? (int16_t)-applied_mag
                                         : (int16_t) applied_mag;
  return state->applied_cmd;
}
