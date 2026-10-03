/*
 * File: BLDC_controller.c
 *
 * Code generated for Simulink model 'BLDC_controller'.
 *
 * Model version                  : 16.64
 * Simulink Coder version         : 25.1 (R2025a) 21-Nov-2024
 * C/C++ source code generated on : Sun Mar 22 19:42:57 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Emulation hardware selection:
 *    Differs from embedded hardware (MATLAB Host)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "BLDC_controller.h"
#include "rtwtypes.h"

/* Named constants for Chart: '<S5>/F03_02_Control_Mode_Manager' */
#define IN_ACTIVE                      ((uint8_T)1U)
#define IN_NO_ACTIVE_CHILD             ((uint8_T)0U)
#define IN_OPEN                        ((uint8_T)2U)
#define IN_SPEED_MODE                  ((uint8_T)1U)
#define IN_TORQUE_MODE                 ((uint8_T)2U)
#define IN_VOLTAGE_MODE                ((uint8_T)3U)
#define OPEN_MODE                      ((uint8_T)0U)
#define SPD_MODE                       ((uint8_T)2U)
#define TRQ_MODE                       ((uint8_T)3U)
#define VLT_MODE                       ((uint8_T)1U)
#ifndef UCHAR_MAX
#include <limits.h>
#endif

#if ( UCHAR_MAX != (0xFFU) ) || ( SCHAR_MAX != (0x7F) )
#error Code was generated for compiler with different sized uchar/char. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( USHRT_MAX != (0xFFFFU) ) || ( SHRT_MAX != (0x7FFF) )
#error Code was generated for compiler with different sized ushort/short. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( UINT_MAX != (0xFFFFFFFFU) ) || ( INT_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized uint/int. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( ULONG_MAX != (0xFFFFFFFFU) ) || ( LONG_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized ulong/long. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

/* Skipping ulong_long/long_long check: insufficient preprocessor integer range. */

/* Exported block parameters */
uint16_T cf_KbLimProt = 246U;          /* Variable: cf_KbLimProt
                                        * Referenced by:
                                        *   '<S220>/cf_KbLimProt'
                                        *   '<S250>/Kb'
                                        */
uint8_T plook_u8u16u16n15_even8ca_gs(uint16_T u, uint16_T bp0, uint16_T
  *fraction);
int16_T intrp1d_s16s32s32u8u16n15la_s(uint8_T bpIndex, uint16_T frac, const
  int16_T table[], uint32_T maxIndex);
uint8_T plook_u8u16_evencka(uint16_T u, uint16_T bp0, uint16_T bpSpace, uint32_T
  maxIndex);
uint8_T plook_u8u16_evenckag(uint16_T u, uint16_T bp0, uint16_T bpSpace);
uint8_T plook_u8s16_evenckag(int16_T u, int16_T bp0, uint16_T bpSpace);
extern void Counter_Init(int16_T rtp_z_cntInit, DW_Counter *localDW);
extern int16_T Counter(int16_T rtu_inc, int16_T rtu_max, boolean_T rtu_rst,
  DW_Counter *localDW);
extern void Low_Pass_Filter_Reset(DW_Low_Pass_Filter *localDW);
extern void Low_Pass_Filter(const int16_T rtu_u[2], uint16_T rtu_coef, int16_T
  rty_y[2], DW_Low_Pass_Filter *localDW);
extern void Counter_e_Init(uint16_T rtp_z_cntInit, DW_Counter_d *localDW);
extern uint16_T Counter_e(uint16_T rtu_inc, uint16_T rtu_max, boolean_T rtu_rst,
  DW_Counter_d *localDW);
extern void Counter_b_Init(uint32_T rtp_z_cntInit, DW_Counter_e *localDW);
extern void Counter_n(uint16_T rtu_inc, uint32_T rtu_max, boolean_T rtu_rst,
                      uint16_T *rty_cnt, DW_Counter_e *localDW);
extern boolean_T either_edge(boolean_T rtu_u, DW_either_edge *localDW);
extern void Debounce_Filter_Init(boolean_T *rty_y, DW_Debounce_Filter *localDW);
extern void Debounce_Filter(boolean_T rtu_u, uint16_T rtu_tAcv, uint32_T
  rtu_tDeacv, boolean_T *rty_y, DW_Debounce_Filter *localDW);
extern void F03_Control_Mode_Manager_Init(uint8_T *rty_z_ctrlMod, int16_T
  *rty_r_inpTgtSca, DW_F03_Control_Mode_Manager *localDW);
extern void F03_Control_Mode_Manager(boolean_T rtu_b_errFlag, boolean_T
  rtu_b_motEna, boolean_T rtu_b_cruiseCtrlEna, uint8_T rtu_z_ctrlTypSel, uint8_T
  rtu_z_ctrlModReq, int16_T rtu_r_inpTgt, int16_T rtu_VqFinPrev, uint8_T
  *rty_z_ctrlMod, int16_T *rty_r_inpTgtScaAbs, int16_T *rty_r_inpTgtSca, int16_T
  *rty_r_inpTgtConv, const P *rtP, DW_F03_Control_Mode_Manager *localDW);
extern void I_backCalc_fixdt_Init(int32_T rtp_yInit, DW_I_backCalc_fixdt
  *localDW);
extern void I_backCalc_fixdt_Reset(int32_T rtp_yInit, DW_I_backCalc_fixdt
  *localDW);
extern void I_backCalc_fixdt(int16_T rtu_err, uint16_T rtu_I, uint16_T rtu_Kb,
  int16_T rtu_satMax, int16_T rtu_satMin, int16_T *rty_out, DW_I_backCalc_fixdt *
  localDW);

#if FeedForwardEnable == 1

extern void PI_clamp_fixdtFF_Init(DW_PI_clamp_fixdtFF *localDW);

#endif

#if FeedForwardEnable == 1

extern void PI_clamp_fixdtFF_Reset(DW_PI_clamp_fixdtFF *localDW);

#endif

#if FeedForwardEnable == 1

extern int16_T PI_clamp_fixdtFF(int16_T rtu_err, uint16_T rtu_P, uint16_T rtu_I,
  int16_T rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int16_T rtu_tgtInput,
  int16_T rtu_FFGain, DW_PI_clamp_fixdtFF *localDW);

#endif

extern void PI_clamp_fixdt_Init(DW_PI_clamp_fixdt *localDW);
extern void PI_clamp_fixdt_Reset(DW_PI_clamp_fixdt *localDW);
extern int16_T PI_clamp_fixdt(int16_T rtu_err, uint16_T rtu_P, uint16_T rtu_I,
  int16_T rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int32_T
  rtu_ext_limProt, DW_PI_clamp_fixdt *localDW);

#if FeedForwardEnable == 1

extern void PI_clamp_fixdtFF_d_Init(DW_PI_clamp_fixdtFF_k *localDW);

#endif

#if FeedForwardEnable == 1

extern void PI_clamp_fixdtFF_f_Reset(DW_PI_clamp_fixdtFF_k *localDW);

#endif

#if FeedForwardEnable == 1

extern int16_T PI_clamp_fixdtFF_c(int16_T rtu_err, uint16_T rtu_P, uint16_T
  rtu_I, int16_T rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int16_T
  rtu_tgtInput, int16_T rtu_FFGain, DW_PI_clamp_fixdtFF_k *localDW);

#endif

extern void FOC_Init(int16_T *rty_Vq, DW_FOC *localDW);
extern void FOC(uint8_T rtu_z_ctrlTypSel, boolean_T rtu_b_enaAdvCtrl, uint8_T
                rtu_z_ctrlMod, int16_T rtu_r_inpTgtSca, int16_T
                rtu_r_inpTgtScaAbs, int16_T rtu_n_mot, int16_T rtu_id_TgtAbs,
                int16_T rtu_id, int16_T rtu_iq, int16_T rtu_Vd_max, int16_T
                rtu_Vd_min, int16_T rtu_Vq_max, int16_T rtu_Vq_min, int16_T
                rtu_id_max, int16_T rtu_id_min, int16_T rtu_iq_max, int16_T
                rtu_iq_min, int16_T rtu_r_iqLimProtMax, int16_T
                rtu_r_nLimProtMax, int32_T rtu_iq_limProtErr, int16_T
                rtu_Vq_nLimProtMax, int16_T rtu_VqFinPrev, boolean_T
                rtu_b_cruiseCtrlEna, const P *rtP, int16_T *rty_Vq, int16_T *rty_Vd, DW_FOC
                *localDW);

/* Forward declaration for local functions */
void enter_internal_ACTIVE(const boolean_T *LogicalOperator1, const boolean_T
  *LogicalOperator2, uint8_T *rty_z_ctrlMod, DW_F03_Control_Mode_Manager
  *localDW);

/*===========*
 * Constants *
 *===========*/
#define RT_PI                          3.14159265358979323846
#define RT_PIF                         3.1415927F
#define RT_LN_10                       2.30258509299404568402
#define RT_LN_10F                      2.3025851F
#define RT_LOG10E                      0.43429448190325182765
#define RT_LOG10EF                     0.43429449F
#define RT_E                           2.7182818284590452354
#define RT_EF                          2.7182817F

/*
 * UNUSED_PARAMETER(x)
 *   Used to specify that a function parameter (argument) is required but not
 *   accessed by the function body.
 */
#ifndef UNUSED_PARAMETER
#if defined(__LCC__)
#define UNUSED_PARAMETER(x)                                      /* do nothing */
#else

/*
 * This is the semi-ANSI standard way of indicating that an
 * unused function parameter is required.
 */
#define UNUSED_PARAMETER(x)            (void) (x)
#endif
#endif

uint8_T plook_u8u16u16n15_even8ca_gs(uint16_T u, uint16_T bp0, uint16_T
  *fraction)
{
  uint16_T uAdjust;
  uint8_T bpIndex;

  /* Prelookup - Index and Fraction
     Index Search method: 'even'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'on'
     Rounding mode: 'simplest'
   */
  uAdjust = (uint16_T)((uint32_T)u - bp0);
  bpIndex = (uint8_T)((uint32_T)uAdjust >> 8U);
  *fraction = (uint16_T)((uint16_T)(uAdjust & 255) << 7);
  return bpIndex;
}

int16_T intrp1d_s16s32s32u8u16n15la_s(uint8_T bpIndex, uint16_T frac, const
  int16_T table[], uint32_T maxIndex)
{
  int16_T y;

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  if (bpIndex == maxIndex) {
    y = table[(uint32_T)bpIndex];
  } else {
    int16_T yL_0d0;
    yL_0d0 = table[(uint32_T)bpIndex];
    y = (int16_T)((int16_T)(((table[bpIndex + 1U] - yL_0d0) * frac) >> 15) +
                  yL_0d0);
  }

  return y;
}

uint8_T plook_u8u16_evencka(uint16_T u, uint16_T bp0, uint16_T bpSpace, uint32_T
  maxIndex)
{
  uint8_T bpIndex;

  /* Prelookup - Index only
     Index Search method: 'even'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u <= bp0) {
    bpIndex = 0U;
  } else {
    uint16_T fbpIndex;
    fbpIndex = (uint16_T)((uint32_T)(uint16_T)((uint32_T)u - bp0) / bpSpace);
    if (fbpIndex < maxIndex) {
      bpIndex = (uint8_T)fbpIndex;
    } else {
      bpIndex = (uint8_T)maxIndex;
    }
  }

  return bpIndex;
}

uint8_T plook_u8u16_evenckag(uint16_T u, uint16_T bp0, uint16_T bpSpace)
{
  /* Prelookup - Index only
     Index Search method: 'even'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'on'
   */
  return (uint8_T)((uint32_T)(uint16_T)((uint32_T)u - bp0) / bpSpace);
}

uint8_T plook_u8s16_evenckag(int16_T u, int16_T bp0, uint16_T bpSpace)
{
  /* Prelookup - Index only
     Index Search method: 'even'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'on'
   */
  return (uint8_T)((uint32_T)(uint16_T)(u - bp0) / bpSpace);
}

/* System initialize for atomic system: '<S13>/Counter' */
void Counter_Init(int16_T rtp_z_cntInit, DW_Counter *localDW)
{
  /* InitializeConditions for UnitDelay: '<S18>/UnitDelay' */
  localDW->UnitDelay_DSTATE = rtp_z_cntInit;
}

/* Output and update for atomic system: '<S13>/Counter' */
int16_T Counter(int16_T rtu_inc, int16_T rtu_max, boolean_T rtu_rst, DW_Counter *
                localDW)
{
  int16_T rty_cnt_0;

  /* UnitDelay: '<S18>/UnitDelay' */
  rty_cnt_0 = localDW->UnitDelay_DSTATE;

  /* Switch: '<S18>/Switch1' incorporates:
   *  Constant: '<S18>/Constant23'
   */
  if (rtu_rst) {
    rty_cnt_0 = 0;
  }

  /* End of Switch: '<S18>/Switch1' */

  /* Sum: '<S16>/Sum1' */
  rty_cnt_0 += rtu_inc;

  /* MinMax: '<S16>/MinMax' */
  if (rty_cnt_0 <= rtu_max) {
    /* Update for UnitDelay: '<S18>/UnitDelay' */
    localDW->UnitDelay_DSTATE = rty_cnt_0;
  } else {
    /* Update for UnitDelay: '<S18>/UnitDelay' */
    localDW->UnitDelay_DSTATE = rtu_max;
  }

  /* End of MinMax: '<S16>/MinMax' */
  return rty_cnt_0;
}

/* System reset for atomic system: '<S66>/Low_Pass_Filter' */
void Low_Pass_Filter_Reset(DW_Low_Pass_Filter *localDW)
{
  /* InitializeConditions for UnitDelay: '<S72>/UnitDelay1' */
  localDW->UnitDelay1_DSTATE[0] = 0;
  localDW->UnitDelay1_DSTATE[1] = 0;
}

/* Output and update for atomic system: '<S66>/Low_Pass_Filter' */
void Low_Pass_Filter(const int16_T rtu_u[2], uint16_T rtu_coef, int16_T rty_y[2],
                     DW_Low_Pass_Filter *localDW)
{
  int32_T rtb_Sum3_c_0;

  /* Sum: '<S72>/Sum2' incorporates:
   *  UnitDelay: '<S72>/UnitDelay1'
   */
  rtb_Sum3_c_0 = rtu_u[0] - (localDW->UnitDelay1_DSTATE[0] >> 16);
  if (rtb_Sum3_c_0 > 32767) {
    rtb_Sum3_c_0 = 32767;
  } else if (rtb_Sum3_c_0 < -32768) {
    rtb_Sum3_c_0 = -32768;
  }

  /* Sum: '<S72>/Sum3' incorporates:
   *  Product: '<S72>/Divide3'
   *  Sum: '<S72>/Sum2'
   *  UnitDelay: '<S72>/UnitDelay1'
   */
  rtb_Sum3_c_0 = rtu_coef * rtb_Sum3_c_0 + localDW->UnitDelay1_DSTATE[0];

  /* DataTypeConversion: '<S72>/Data Type Conversion' incorporates:
   *  Sum: '<S72>/Sum3'
   */
  rty_y[0] = (int16_T)(rtb_Sum3_c_0 >> 16);

  /* Update for UnitDelay: '<S72>/UnitDelay1' incorporates:
   *  Sum: '<S72>/Sum3'
   */
  localDW->UnitDelay1_DSTATE[0] = rtb_Sum3_c_0;

  /* Sum: '<S72>/Sum2' incorporates:
   *  UnitDelay: '<S72>/UnitDelay1'
   */
  rtb_Sum3_c_0 = rtu_u[1] - (localDW->UnitDelay1_DSTATE[1] >> 16);
  if (rtb_Sum3_c_0 > 32767) {
    rtb_Sum3_c_0 = 32767;
  } else if (rtb_Sum3_c_0 < -32768) {
    rtb_Sum3_c_0 = -32768;
  }

  /* Sum: '<S72>/Sum3' incorporates:
   *  Product: '<S72>/Divide3'
   *  Sum: '<S72>/Sum2'
   *  UnitDelay: '<S72>/UnitDelay1'
   */
  rtb_Sum3_c_0 = rtu_coef * rtb_Sum3_c_0 + localDW->UnitDelay1_DSTATE[1];

  /* DataTypeConversion: '<S72>/Data Type Conversion' incorporates:
   *  Sum: '<S72>/Sum3'
   */
  rty_y[1] = (int16_T)(rtb_Sum3_c_0 >> 16);

  /* Update for UnitDelay: '<S72>/UnitDelay1' incorporates:
   *  Sum: '<S72>/Sum3'
   */
  localDW->UnitDelay1_DSTATE[1] = rtb_Sum3_c_0;
}

/* System initialize for atomic system: '<S38>/Counter' */
void Counter_e_Init(uint16_T rtp_z_cntInit, DW_Counter_d *localDW)
{
  /* InitializeConditions for UnitDelay: '<S43>/UnitDelay' */
  localDW->UnitDelay_DSTATE = rtp_z_cntInit;
}

/* Output and update for atomic system: '<S38>/Counter' */
uint16_T Counter_e(uint16_T rtu_inc, uint16_T rtu_max, boolean_T rtu_rst,
                   DW_Counter_d *localDW)
{
  uint16_T rty_cnt_0;
  uint16_T tmp;

  /* Switch: '<S43>/Switch1' incorporates:
   *  Constant: '<S43>/Constant23'
   *  UnitDelay: '<S43>/UnitDelay'
   */
  if (rtu_rst) {
    tmp = 0U;
  } else {
    tmp = localDW->UnitDelay_DSTATE;
  }

  /* Sum: '<S42>/Sum1' incorporates:
   *  Switch: '<S43>/Switch1'
   */
  rty_cnt_0 = (uint16_T)(rtu_inc + tmp);

  /* MinMax: '<S42>/MinMax' */
  if (rty_cnt_0 <= rtu_max) {
    /* Update for UnitDelay: '<S43>/UnitDelay' */
    localDW->UnitDelay_DSTATE = rty_cnt_0;
  } else {
    /* Update for UnitDelay: '<S43>/UnitDelay' */
    localDW->UnitDelay_DSTATE = rtu_max;
  }

  /* End of MinMax: '<S42>/MinMax' */
  return rty_cnt_0;
}

/* System initialize for atomic system: '<S37>/Counter' */
void Counter_b_Init(uint32_T rtp_z_cntInit, DW_Counter_e *localDW)
{
  /* InitializeConditions for UnitDelay: '<S41>/UnitDelay' */
  localDW->UnitDelay_DSTATE = rtp_z_cntInit;
}

/* Output and update for atomic system: '<S37>/Counter' */
void Counter_n(uint16_T rtu_inc, uint32_T rtu_max, boolean_T rtu_rst, uint16_T
               *rty_cnt, DW_Counter_e *localDW)
{
  uint16_T tmp;

  /* Switch: '<S41>/Switch1' incorporates:
   *  Constant: '<S41>/Constant23'
   *  UnitDelay: '<S41>/UnitDelay'
   */
  if (rtu_rst) {
    tmp = 0U;
  } else {
    tmp = (uint16_T)localDW->UnitDelay_DSTATE;
  }

  /* Sum: '<S40>/Sum1' incorporates:
   *  Switch: '<S41>/Switch1'
   */
  *rty_cnt = (uint16_T)(rtu_inc + tmp);

  /* MinMax: '<S40>/MinMax' */
  if (*rty_cnt <= rtu_max) {
    /* Update for UnitDelay: '<S41>/UnitDelay' */
    localDW->UnitDelay_DSTATE = *rty_cnt;
  } else {
    /* Update for UnitDelay: '<S41>/UnitDelay' */
    localDW->UnitDelay_DSTATE = rtu_max;
  }

  /* End of MinMax: '<S40>/MinMax' */
}

/*
 * Output and update for atomic system:
 *    '<S34>/either_edge'
 *    '<S33>/either_edge'
 */
boolean_T either_edge(boolean_T rtu_u, DW_either_edge *localDW)
{
  boolean_T rty_y_0;

  /* RelationalOperator: '<S39>/Relational Operator' incorporates:
   *  UnitDelay: '<S39>/UnitDelay'
   */
  rty_y_0 = (rtu_u != localDW->UnitDelay_DSTATE);

  /* Update for UnitDelay: '<S39>/UnitDelay' */
  localDW->UnitDelay_DSTATE = rtu_u;
  return rty_y_0;
}

/* System initialize for atomic system: '<S33>/Debounce_Filter' */
void Debounce_Filter_Init(boolean_T *rty_y, DW_Debounce_Filter *localDW)
{
  /* SystemInitialize for IfAction SubSystem: '<S34>/Qualification' */
  /* SystemInitialize for Atomic SubSystem: '<S38>/Counter' */
  Counter_e_Init(0, &localDW->Counter_e2);

  /* End of SystemInitialize for SubSystem: '<S38>/Counter' */
  /* End of SystemInitialize for SubSystem: '<S34>/Qualification' */

  /* SystemInitialize for IfAction SubSystem: '<S34>/Dequalification' */
  /* SystemInitialize for Atomic SubSystem: '<S37>/Counter' */
  Counter_b_Init(0U, &localDW->Counter_ni);

  /* End of SystemInitialize for SubSystem: '<S37>/Counter' */
  /* End of SystemInitialize for SubSystem: '<S34>/Dequalification' */

  /* SystemInitialize for Merge: '<S34>/Merge' */
  *rty_y = false;
}

/* Output and update for atomic system: '<S33>/Debounce_Filter' */
void Debounce_Filter(boolean_T rtu_u, uint16_T rtu_tAcv, uint32_T rtu_tDeacv,
                     boolean_T *rty_y, DW_Debounce_Filter *localDW)
{
  uint16_T rtb_Sum1_i0;
  boolean_T rtb_RelationalOperator_o;

  /* Outputs for Atomic SubSystem: '<S34>/either_edge' */
  rtb_RelationalOperator_o = either_edge(rtu_u, &localDW->either_edge_n);

  /* End of Outputs for SubSystem: '<S34>/either_edge' */

  /* If: '<S34>/If2' incorporates:
   *  Constant: '<S37>/Constant6'
   *  Constant: '<S38>/Constant6'
   *  Logic: '<S34>/Logical Operator1'
   *  Logic: '<S34>/Logical Operator2'
   *  Logic: '<S34>/Logical Operator3'
   *  Logic: '<S34>/Logical Operator4'
   *  UnitDelay: '<S34>/UnitDelay'
   */
  if (rtu_u && (!localDW->UnitDelay_DSTATE)) {
    /* Outputs for IfAction SubSystem: '<S34>/Qualification' incorporates:
     *  ActionPort: '<S38>/Action Port'
     */
    /* Outputs for Atomic SubSystem: '<S38>/Counter' */
    rtb_Sum1_i0 = Counter_e(1, rtu_tAcv, rtb_RelationalOperator_o,
      &localDW->Counter_e2);

    /* End of Outputs for SubSystem: '<S38>/Counter' */

    /* Switch: '<S38>/Switch2' incorporates:
     *  Constant: '<S38>/Constant6'
     *  RelationalOperator: '<S38>/Relational Operator2'
     */
    *rty_y = (rtb_Sum1_i0 > rtu_tAcv);

    /* End of Outputs for SubSystem: '<S34>/Qualification' */
  } else if ((!rtu_u) && localDW->UnitDelay_DSTATE) {
    /* Outputs for IfAction SubSystem: '<S34>/Dequalification' incorporates:
     *  ActionPort: '<S37>/Action Port'
     */
    /* Outputs for Atomic SubSystem: '<S37>/Counter' */
    Counter_n(1, rtu_tDeacv, rtb_RelationalOperator_o, &rtb_Sum1_i0,
              &localDW->Counter_ni);

    /* End of Outputs for SubSystem: '<S37>/Counter' */

    /* Switch: '<S37>/Switch2' incorporates:
     *  Constant: '<S37>/Constant6'
     *  RelationalOperator: '<S37>/Relational Operator2'
     */
    *rty_y = (rtb_Sum1_i0 <= rtu_tDeacv);

    /* End of Outputs for SubSystem: '<S34>/Dequalification' */
  } else {
    /* Outputs for IfAction SubSystem: '<S34>/Default' incorporates:
     *  ActionPort: '<S36>/Action Port'
     */
    /* SignalConversion generated from: '<S36>/yPrev' */
    *rty_y = localDW->UnitDelay_DSTATE;

    /* End of Outputs for SubSystem: '<S34>/Default' */
  }

  /* End of If: '<S34>/If2' */

  /* Update for UnitDelay: '<S34>/UnitDelay' */
  localDW->UnitDelay_DSTATE = *rty_y;
}

/* Function for Chart: '<S5>/F03_02_Control_Mode_Manager' */
void enter_internal_ACTIVE(const boolean_T *LogicalOperator1, const boolean_T
  *LogicalOperator2, uint8_T *rty_z_ctrlMod, DW_F03_Control_Mode_Manager
  *localDW)
{
  if (*LogicalOperator2) {
    localDW->is_ACTIVE = IN_TORQUE_MODE;

    /* Chart: '<S5>/F03_02_Control_Mode_Manager' */
    *rty_z_ctrlMod = TRQ_MODE;
  } else if (*LogicalOperator1) {
    localDW->is_ACTIVE = IN_SPEED_MODE;

    /* Chart: '<S5>/F03_02_Control_Mode_Manager' */
    *rty_z_ctrlMod = SPD_MODE;
  } else {
    localDW->is_ACTIVE = IN_VOLTAGE_MODE;

    /* Chart: '<S5>/F03_02_Control_Mode_Manager' */
    *rty_z_ctrlMod = VLT_MODE;
  }
}

/* System initialize for function-call system: '<S1>/F03_Control_Mode_Manager' */
void F03_Control_Mode_Manager_Init(uint8_T *rty_z_ctrlMod, int16_T
  *rty_r_inpTgtSca, DW_F03_Control_Mode_Manager *localDW)
{
  /* Start for If: '<S46>/If2' */
  localDW->If2_ActiveSubsystem = -1;

  /* SystemInitialize for Chart: '<S5>/F03_02_Control_Mode_Manager' */
  *rty_z_ctrlMod = 0U;

  /* SystemInitialize for IfAction SubSystem: '<S46>/Open_Mode' */
  /* SystemInitialize for Atomic SubSystem: '<S50>/rising_edge_init' */
  /* InitializeConditions for UnitDelay: '<S55>/UnitDelay' */
  localDW->UnitDelay_DSTATE_c = true;

  /* End of SystemInitialize for SubSystem: '<S50>/rising_edge_init' */
  /* End of SystemInitialize for SubSystem: '<S46>/Open_Mode' */

  /* SystemInitialize for Merge: '<S46>/Merge1' */
  *rty_r_inpTgtSca = 0;
}

/* Output and update for function-call system: '<S1>/F03_Control_Mode_Manager' */
void F03_Control_Mode_Manager(boolean_T rtu_b_errFlag, boolean_T rtu_b_motEna,
  boolean_T rtu_b_cruiseCtrlEna, uint8_T rtu_z_ctrlTypSel, uint8_T
  rtu_z_ctrlModReq, int16_T rtu_r_inpTgt, int16_T rtu_VqFinPrev, uint8_T
  *rty_z_ctrlMod, int16_T *rty_r_inpTgtScaAbs, int16_T *rty_r_inpTgtSca, int16_T
  *rty_r_inpTgtConv, const P *rtP, DW_F03_Control_Mode_Manager *localDW)
{
  int32_T rtb_DataTypeConversion;
  int32_T rtb_Gain3;
  int32_T rtb_Sum1_ae;
  int32_T rtb_Switch1_c;
  int16_T tmp[4];
  int16_T rtb_VariantMergeForOutportVd__g;
  int8_T rtAction;
  int8_T rtPrevAction;
  boolean_T LogicalOperator1;
  boolean_T LogicalOperator2;
  boolean_T LogicalOperator4;

  /* Logic: '<S44>/Logical Operator4' incorporates:
   *  Constant: '<S44>/constant8'
   *  Logic: '<S44>/Logical Operator7'
   *  RelationalOperator: '<S44>/Relational Operator10'
   */
  LogicalOperator4 = (rtu_b_errFlag || (!rtu_b_motEna) || (rtu_z_ctrlModReq == 0));

  /* Logic: '<S44>/Logical Operator1' incorporates:
   *  Constant: '<S44>/constant1'
   *  RelationalOperator: '<S44>/Relational Operator1'
   */
  LogicalOperator1 = ((rtu_z_ctrlModReq == 2) || rtu_b_cruiseCtrlEna);

  /* Logic: '<S44>/Logical Operator2' incorporates:
   *  Constant: '<S44>/constant'
   *  Logic: '<S44>/Logical Operator5'
   *  RelationalOperator: '<S44>/Relational Operator4'
   */
  LogicalOperator2 = ((rtu_z_ctrlModReq == 3) && (!rtu_b_cruiseCtrlEna));

  /* Chart: '<S5>/F03_02_Control_Mode_Manager' incorporates:
   *  Constant: '<S44>/constant5'
   *  Logic: '<S44>/Logical Operator3'
   *  Logic: '<S44>/Logical Operator6'
   *  Logic: '<S44>/Logical Operator9'
   *  RelationalOperator: '<S44>/Relational Operator5'
   */
  if (localDW->is_active_c2_BLDC_controller == 0) {
    localDW->is_active_c2_BLDC_controller = 1U;
    localDW->is_c2_BLDC_controller = IN_OPEN;
    *rty_z_ctrlMod = OPEN_MODE;
  } else if (localDW->is_c2_BLDC_controller == IN_ACTIVE) {
    if (LogicalOperator4) {
      localDW->is_ACTIVE = IN_NO_ACTIVE_CHILD;
      localDW->is_c2_BLDC_controller = IN_OPEN;
      *rty_z_ctrlMod = OPEN_MODE;
    } else {
      switch (localDW->is_ACTIVE) {
       case IN_SPEED_MODE:
        *rty_z_ctrlMod = SPD_MODE;
        if (!LogicalOperator1) {
          enter_internal_ACTIVE(&LogicalOperator1, &LogicalOperator2,
                                rty_z_ctrlMod, localDW);
        }
        break;

       case IN_TORQUE_MODE:
        *rty_z_ctrlMod = TRQ_MODE;
        if (!LogicalOperator2) {
          enter_internal_ACTIVE(&LogicalOperator1, &LogicalOperator2,
                                rty_z_ctrlMod, localDW);
        }
        break;

       default:
        /* case IN_VOLTAGE_MODE: */
        *rty_z_ctrlMod = VLT_MODE;
        if (LogicalOperator2 || LogicalOperator1) {
          enter_internal_ACTIVE(&LogicalOperator1, &LogicalOperator2,
                                rty_z_ctrlMod, localDW);
        }
        break;
      }
    }
  } else {
    /* case IN_OPEN: */
    *rty_z_ctrlMod = OPEN_MODE;
    if ((!LogicalOperator4) && ((rtu_z_ctrlModReq == 1) || LogicalOperator1 ||
         LogicalOperator2)) {
      localDW->is_c2_BLDC_controller = IN_ACTIVE;
      enter_internal_ACTIVE(&LogicalOperator1, &LogicalOperator2, rty_z_ctrlMod,
                            localDW);
    }
  }

  /* End of Chart: '<S5>/F03_02_Control_Mode_Manager' */

  /* DataTypeConversion: '<S46>/Data Type Conversion22' */
  *rty_r_inpTgtConv = rtu_r_inpTgt;

  /* If: '<S46>/If1' incorporates:
   *  Constant: '<S52>/Vd_max1'
   *  Constant: '<S53>/Vd_max3'
   *  VariantMerge generated from: '<S51>/Vd_max_margin'
   */
  if (rtu_z_ctrlTypSel == 2) {
    /* Outputs for IfAction SubSystem: '<S46>/FOC_Control_Type' incorporates:
     *  ActionPort: '<S49>/Action Port'
     */
    /* Outputs for Atomic SubSystem: '<S49>/Variant Subsystem1' */
#if mcu_model == 1

    /* Outputs for Atomic SubSystem: '<S51>/GD32F103' */
    rtb_VariantMergeForOutportVd__g = 26032;

    /* End of Outputs for SubSystem: '<S51>/GD32F103' */
#elif mcu_model == 0

    /* Outputs for Atomic SubSystem: '<S51>/STM32F103' */
    rtb_VariantMergeForOutportVd__g = 15040;

    /* End of Outputs for SubSystem: '<S51>/STM32F103' */
#endif

    /* End of Outputs for SubSystem: '<S49>/Variant Subsystem1' */

    /* SignalConversion generated from: '<S49>/Selector' incorporates:
     *  Constant: '<S49>/constant1'
     *  Constant: '<S49>/i_max'
     *  Constant: '<S49>/n_max'
     *  Constant: '<S52>/Vd_max1'
     *  Constant: '<S53>/Vd_max3'
     *  VariantMerge generated from: '<S51>/Vd_max_margin'
     */
    tmp[0] = 0;
    tmp[1] = rtb_VariantMergeForOutportVd__g;
    tmp[2] = rtP->n_max;
    tmp[3] = rtP->i_max;

    /* Product: '<S49>/Divide1' incorporates:
     *  Product: '<S49>/Divide4'
     *  Selector: '<S49>/Selector'
     */
    *rty_r_inpTgtSca = (int16_T)(((uint16_T)((tmp[rtu_z_ctrlModReq] << 10) /
      2047) * *rty_r_inpTgtConv) >> 14);

    /* End of Outputs for SubSystem: '<S46>/FOC_Control_Type' */
  } else {
    /* Outputs for IfAction SubSystem: '<S46>/Default_Control_Type' incorporates:
     *  ActionPort: '<S47>/Action Port'
     */
    /* SignalConversion generated from: '<S47>/r_inpTgtConv' */
    *rty_r_inpTgtSca = *rty_r_inpTgtConv;

    /* End of Outputs for SubSystem: '<S46>/Default_Control_Type' */
  }

  /* End of If: '<S46>/If1' */

  /* If: '<S46>/If2' */
  rtPrevAction = localDW->If2_ActiveSubsystem;
  rtAction = (int8_T)(*rty_z_ctrlMod != 0);
  localDW->If2_ActiveSubsystem = rtAction;
  if (rtAction == 0) {
    if (rtPrevAction != 0) {
      /* SystemReset for IfAction SubSystem: '<S46>/Open_Mode' incorporates:
       *  ActionPort: '<S50>/Action Port'
       */
      /* SystemReset for Atomic SubSystem: '<S50>/rising_edge_init' */
      /* SystemReset for If: '<S46>/If2' incorporates:
       *  UnitDelay: '<S55>/UnitDelay'
       *  UnitDelay: '<S56>/UnitDelay'
       */
      localDW->UnitDelay_DSTATE_c = true;

      /* End of SystemReset for SubSystem: '<S50>/rising_edge_init' */

      /* SystemReset for Atomic SubSystem: '<S50>/Rate_Limiter' */
      localDW->UnitDelay_DSTATE = 0;

      /* End of SystemReset for SubSystem: '<S50>/Rate_Limiter' */
      /* End of SystemReset for SubSystem: '<S46>/Open_Mode' */
    }

    /* Outputs for IfAction SubSystem: '<S46>/Open_Mode' incorporates:
     *  ActionPort: '<S50>/Action Port'
     */
    /* DataTypeConversion: '<S50>/Data Type Conversion' */
    rtb_DataTypeConversion = rtu_VqFinPrev << 12;

    /* Outputs for Atomic SubSystem: '<S50>/rising_edge_init' */
    /* UnitDelay: '<S55>/UnitDelay' */
    LogicalOperator4 = localDW->UnitDelay_DSTATE_c;

    /* Update for UnitDelay: '<S55>/UnitDelay' incorporates:
     *  Constant: '<S55>/Constant'
     */
    localDW->UnitDelay_DSTATE_c = false;

    /* End of Outputs for SubSystem: '<S50>/rising_edge_init' */

    /* Outputs for Atomic SubSystem: '<S50>/Rate_Limiter' */
    /* Switch: '<S56>/Switch1' incorporates:
     *  DataTypeConversion: '<S50>/Data Type Conversion'
     *  UnitDelay: '<S56>/UnitDelay'
     */
    if (LogicalOperator4) {
      rtb_Switch1_c = rtb_DataTypeConversion;
    } else {
      rtb_Switch1_c = localDW->UnitDelay_DSTATE;
    }

    /* End of Switch: '<S56>/Switch1' */

    /* Sum: '<S54>/Sum1' incorporates:
     *  Switch: '<S56>/Switch1'
     */
    rtb_Sum1_ae = ((uint32_T)-rtb_Switch1_c & 134217728U) != 0U ? -rtb_Switch1_c
      | -134217728 : (int32_T)((uint32_T)-rtb_Switch1_c & 134217727U);

    /* Switch: '<S57>/Switch2' incorporates:
     *  Constant: '<S50>/dV_openRate'
     *  RelationalOperator: '<S57>/LowerRelop1'
     *  Sum: '<S54>/Sum1'
     */
    if (rtb_Sum1_ae > rtP->dV_openRate) {
      rtb_Sum1_ae = rtP->dV_openRate;
    } else {
      /* Gain: '<S50>/Gain3' */
      rtb_Gain3 = ((uint32_T)-rtP->dV_openRate & 134217728U) != 0U ?
        -rtP->dV_openRate | -134217728 : (int32_T)((uint32_T)-rtP->dV_openRate &
        134217727U);

      /* Switch: '<S57>/Switch' incorporates:
       *  Gain: '<S50>/Gain3'
       *  RelationalOperator: '<S57>/UpperRelop'
       *  Switch: '<S57>/Switch2'
       */
      if (rtb_Sum1_ae < rtb_Gain3) {
        rtb_Sum1_ae = rtb_Gain3;
      }

      /* End of Switch: '<S57>/Switch' */
    }

    /* End of Switch: '<S57>/Switch2' */

    /* Sum: '<S54>/Sum2' incorporates:
     *  Switch: '<S56>/Switch1'
     *  Switch: '<S57>/Switch2'
     */
    rtb_Switch1_c += rtb_Sum1_ae;
    rtb_Switch1_c = ((uint32_T)rtb_Switch1_c & 134217728U) != 0U ? rtb_Switch1_c
      | -134217728 : (int32_T)((uint32_T)rtb_Switch1_c & 134217727U);

    /* Switch: '<S56>/Switch2' */
    if (LogicalOperator4) {
      /* Update for UnitDelay: '<S56>/UnitDelay' incorporates:
       *  DataTypeConversion: '<S50>/Data Type Conversion'
       */
      localDW->UnitDelay_DSTATE = rtb_DataTypeConversion;
    } else {
      /* Update for UnitDelay: '<S56>/UnitDelay' incorporates:
       *  Sum: '<S54>/Sum2'
       */
      localDW->UnitDelay_DSTATE = rtb_Switch1_c;
    }

    /* End of Switch: '<S56>/Switch2' */
    /* End of Outputs for SubSystem: '<S50>/Rate_Limiter' */

    /* DataTypeConversion: '<S50>/Data Type Conversion1' incorporates:
     *  Sum: '<S54>/Sum2'
     */
    *rty_r_inpTgtSca = (int16_T)(rtb_Switch1_c >> 12);

    /* End of Outputs for SubSystem: '<S46>/Open_Mode' */
  }

  /* End of If: '<S46>/If2' */

  /* Abs: '<S5>/Abs1' */
  if (*rty_r_inpTgtSca < 0) {
    *rty_r_inpTgtScaAbs = (int16_T)-*rty_r_inpTgtSca;
  } else {
    *rty_r_inpTgtScaAbs = *rty_r_inpTgtSca;
  }

  /* End of Abs: '<S5>/Abs1' */
}

/*
 * System initialize for atomic system:
 *    '<S220>/I_backCalc_fixdt'
 *    '<S220>/I_backCalc_fixdt1'
 */
void I_backCalc_fixdt_Init(int32_T rtp_yInit, DW_I_backCalc_fixdt *localDW)
{
  /* InitializeConditions for UnitDelay: '<S283>/UnitDelay' */
  localDW->UnitDelay_DSTATE_a = rtp_yInit;
}

/*
 * System reset for atomic system:
 *    '<S220>/I_backCalc_fixdt'
 *    '<S220>/I_backCalc_fixdt1'
 */
void I_backCalc_fixdt_Reset(int32_T rtp_yInit, DW_I_backCalc_fixdt *localDW)
{
  /* InitializeConditions for UnitDelay: '<S281>/UnitDelay' */
  localDW->UnitDelay_DSTATE = 0;

  /* InitializeConditions for UnitDelay: '<S283>/UnitDelay' */
  localDW->UnitDelay_DSTATE_a = rtp_yInit;
}

/*
 * Output and update for atomic system:
 *    '<S220>/I_backCalc_fixdt'
 *    '<S220>/I_backCalc_fixdt1'
 */
void I_backCalc_fixdt(int16_T rtu_err, uint16_T rtu_I, uint16_T rtu_Kb, int16_T
                      rtu_satMax, int16_T rtu_satMin, int16_T *rty_out,
                      DW_I_backCalc_fixdt *localDW)
{
  int64_T tmp;
  int32_T rtb_Sum1_gm;
  int16_T rtb_DataTypeConversion1_l;

  /* Sum: '<S281>/Sum2' incorporates:
   *  Product: '<S281>/Divide2'
   *  UnitDelay: '<S281>/UnitDelay'
   */
  tmp = (rtu_err * rtu_I + ((int64_T)localDW->UnitDelay_DSTATE << 4)) >> 4;
  if (tmp > 2147483647LL) {
    tmp = 2147483647LL;
  } else if (tmp < -2147483648LL) {
    tmp = -2147483648LL;
  }

  /* Sum: '<S283>/Sum1' incorporates:
   *  Sum: '<S281>/Sum2'
   *  UnitDelay: '<S283>/UnitDelay'
   */
  rtb_Sum1_gm = (int32_T)tmp + localDW->UnitDelay_DSTATE_a;

  /* DataTypeConversion: '<S283>/Data Type Conversion1' incorporates:
   *  Sum: '<S283>/Sum1'
   */
  rtb_DataTypeConversion1_l = (int16_T)(rtb_Sum1_gm >> 12);

  /* Switch: '<S284>/Switch2' incorporates:
   *  DataTypeConversion: '<S283>/Data Type Conversion1'
   *  RelationalOperator: '<S284>/LowerRelop1'
   *  RelationalOperator: '<S284>/UpperRelop'
   *  Switch: '<S284>/Switch'
   */
  if (rtb_DataTypeConversion1_l > rtu_satMax) {
    *rty_out = rtu_satMax;
  } else if (rtb_DataTypeConversion1_l < rtu_satMin) {
    /* Switch: '<S284>/Switch' */
    *rty_out = rtu_satMin;
  } else {
    *rty_out = rtb_DataTypeConversion1_l;
  }

  /* End of Switch: '<S284>/Switch2' */

  /* Update for UnitDelay: '<S281>/UnitDelay' incorporates:
   *  DataTypeConversion: '<S283>/Data Type Conversion1'
   *  Product: '<S281>/Divide1'
   *  Sum: '<S281>/Sum3'
   */
  localDW->UnitDelay_DSTATE = (int16_T)(*rty_out - rtb_DataTypeConversion1_l) *
    rtu_Kb;

  /* Update for UnitDelay: '<S283>/UnitDelay' incorporates:
   *  Sum: '<S283>/Sum1'
   */
  localDW->UnitDelay_DSTATE_a = rtb_Sum1_gm;
}

/* System initialize for atomic system: '<S153>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

void PI_clamp_fixdtFF_Init(DW_PI_clamp_fixdtFF *localDW)
{
  /* InitializeConditions for Delay: '<S213>/Resettable Delay' */
  localDW->icLoad = true;
}

#endif

/* System reset for atomic system: '<S153>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

void PI_clamp_fixdtFF_Reset(DW_PI_clamp_fixdtFF *localDW)
{
  /* InitializeConditions for UnitDelay: '<S211>/UnitDelay1' */
  localDW->UnitDelay1_DSTATE = false;

  /* InitializeConditions for Delay: '<S213>/Resettable Delay' */
  localDW->icLoad = true;
}

#endif

/* Output and update for atomic system: '<S153>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

int16_T PI_clamp_fixdtFF(int16_T rtu_err, uint16_T rtu_P, uint16_T rtu_I,
  int16_T rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int16_T rtu_tgtInput,
  int16_T rtu_FFGain, DW_PI_clamp_fixdtFF *localDW)
{
  int16_T rty_out_0;
  int32_T rtb_SignDeltaU2_c;
  int32_T rtb_Sum1_l;
  int32_T tmp;
  int32_T tmp_0;
  int16_T tmp_1;
  boolean_T rtb_LowerRelop1_bq;
  boolean_T rtb_UpperRelop_gv;

  /* Product: '<S211>/Divide2' incorporates:
   *  Signum: '<S212>/SignDeltaU2'
   */
  rtb_SignDeltaU2_c = rtu_err * rtu_I;

  /* Delay: '<S213>/Resettable Delay' */
  if (localDW->icLoad) {
    localDW->ResettableDelay_DSTATE = rtu_init << 16;
  }

  /* Switch: '<S211>/Switch1' incorporates:
   *  Constant: '<S211>/Constant'
   *  Signum: '<S212>/SignDeltaU2'
   *  UnitDelay: '<S211>/UnitDelay1'
   */
  if (localDW->UnitDelay1_DSTATE) {
    tmp_0 = 0;
  } else {
    tmp_0 = rtb_SignDeltaU2_c;
  }

  /* Sum: '<S213>/Sum1' incorporates:
   *  Delay: '<S213>/Resettable Delay'
   *  Switch: '<S211>/Switch1'
   */
  rtb_Sum1_l = tmp_0 + localDW->ResettableDelay_DSTATE;

  /* Product: '<S211>/Product' */
  tmp_0 = (rtu_FFGain * rtu_tgtInput) >> 12;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* Product: '<S211>/Divide5' */
  tmp = (rtu_err * rtu_P) >> 9;
  if (tmp > 32767) {
    tmp = 32767;
  } else if (tmp < -32768) {
    tmp = -32768;
  }

  /* Sum: '<S211>/Sum1' incorporates:
   *  DataTypeConversion: '<S213>/Data Type Conversion1'
   *  Product: '<S211>/Divide5'
   *  Product: '<S211>/Product'
   *  Sum: '<S213>/Sum1'
   */
  tmp_0 = (((tmp_0 << 1) + tmp) + ((rtb_Sum1_l >> 16) << 1)) >> 1;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* RelationalOperator: '<S214>/LowerRelop1' incorporates:
   *  Sum: '<S211>/Sum1'
   */
  rtb_LowerRelop1_bq = (tmp_0 > rtu_satMax);

  /* RelationalOperator: '<S214>/UpperRelop' incorporates:
   *  Sum: '<S211>/Sum1'
   */
  rtb_UpperRelop_gv = (tmp_0 < rtu_satMin);

  /* Switch: '<S214>/Switch1' incorporates:
   *  Sum: '<S211>/Sum1'
   *  Switch: '<S214>/Switch3'
   */
  if (rtb_LowerRelop1_bq) {
    rty_out_0 = rtu_satMax;
  } else if (rtb_UpperRelop_gv) {
    /* Switch: '<S214>/Switch3' */
    rty_out_0 = rtu_satMin;
  } else {
    rty_out_0 = (int16_T)tmp_0;
  }

  /* End of Switch: '<S214>/Switch1' */

  /* Signum: '<S212>/SignDeltaU2' */
  if (rtb_SignDeltaU2_c < 0) {
    tmp = -1;
  } else {
    tmp = (rtb_SignDeltaU2_c > 0);
  }

  /* Signum: '<S212>/SignDeltaU3' incorporates:
   *  Sum: '<S211>/Sum1'
   */
  if (tmp_0 < 0) {
    tmp_1 = -1;
  } else {
    tmp_1 = (int16_T)(tmp_0 > 0);
  }

  /* Update for UnitDelay: '<S211>/UnitDelay1' incorporates:
   *  Logic: '<S211>/AND1'
   *  Logic: '<S212>/AND1'
   *  RelationalOperator: '<S212>/Equal1'
   *  Signum: '<S212>/SignDeltaU2'
   *  Signum: '<S212>/SignDeltaU3'
   */
  localDW->UnitDelay1_DSTATE = ((tmp == tmp_1) && (rtb_LowerRelop1_bq ||
    rtb_UpperRelop_gv));

  /* Update for Delay: '<S213>/Resettable Delay' incorporates:
   *  Sum: '<S213>/Sum1'
   */
  localDW->icLoad = false;
  localDW->ResettableDelay_DSTATE = rtb_Sum1_l;
  return rty_out_0;
}

#endif

/* System initialize for atomic system: '<S77>/PI_clamp_fixdt' */
void PI_clamp_fixdt_Init(DW_PI_clamp_fixdt *localDW)
{
  /* InitializeConditions for Delay: '<S83>/Resettable Delay' */
  localDW->icLoad = true;
}

/* System reset for atomic system: '<S77>/PI_clamp_fixdt' */
void PI_clamp_fixdt_Reset(DW_PI_clamp_fixdt *localDW)
{
  /* InitializeConditions for UnitDelay: '<S81>/UnitDelay1' */
  localDW->UnitDelay1_DSTATE = false;

  /* InitializeConditions for Delay: '<S83>/Resettable Delay' */
  localDW->icLoad = true;
}

/* Output and update for atomic system: '<S77>/PI_clamp_fixdt' */
int16_T PI_clamp_fixdt(int16_T rtu_err, uint16_T rtu_P, uint16_T rtu_I, int16_T
  rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int32_T rtu_ext_limProt,
  DW_PI_clamp_fixdt *localDW)
{
  int16_T rty_out_0;
  int64_T tmp;
  int32_T rtb_Sum1_d;
  int32_T tmp_0;
  int32_T tmp_1;
  int16_T tmp_2;
  boolean_T rtb_LowerRelop1_ow;
  boolean_T rtb_UpperRelop_f3;

  /* Sum: '<S81>/Sum2' incorporates:
   *  Product: '<S81>/Divide2'
   */
  tmp = (int64_T)(rtu_err * rtu_I) + rtu_ext_limProt;
  if (tmp > 2147483647LL) {
    tmp = 2147483647LL;
  } else if (tmp < -2147483648LL) {
    tmp = -2147483648LL;
  }

  /* Delay: '<S83>/Resettable Delay' */
  if (localDW->icLoad) {
    localDW->ResettableDelay_DSTATE = rtu_init << 16;
  }

  /* Switch: '<S81>/Switch1' incorporates:
   *  Constant: '<S81>/Constant'
   *  Sum: '<S81>/Sum2'
   *  UnitDelay: '<S81>/UnitDelay1'
   */
  if (localDW->UnitDelay1_DSTATE) {
    tmp_0 = 0;
  } else {
    tmp_0 = (int32_T)tmp;
  }

  /* Sum: '<S83>/Sum1' incorporates:
   *  Delay: '<S83>/Resettable Delay'
   *  Switch: '<S81>/Switch1'
   */
  rtb_Sum1_d = tmp_0 + localDW->ResettableDelay_DSTATE;

  /* Product: '<S81>/Divide5' */
  tmp_0 = (rtu_err * rtu_P) >> 11;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* Sum: '<S81>/Sum1' incorporates:
   *  DataTypeConversion: '<S83>/Data Type Conversion1'
   *  Product: '<S81>/Divide5'
   *  Sum: '<S83>/Sum1'
   */
  tmp_0 = (((rtb_Sum1_d >> 16) << 1) + tmp_0) >> 1;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* RelationalOperator: '<S84>/LowerRelop1' incorporates:
   *  Sum: '<S81>/Sum1'
   */
  rtb_LowerRelop1_ow = (tmp_0 > rtu_satMax);

  /* RelationalOperator: '<S84>/UpperRelop' incorporates:
   *  Sum: '<S81>/Sum1'
   */
  rtb_UpperRelop_f3 = (tmp_0 < rtu_satMin);

  /* Switch: '<S84>/Switch1' incorporates:
   *  Sum: '<S81>/Sum1'
   *  Switch: '<S84>/Switch3'
   */
  if (rtb_LowerRelop1_ow) {
    rty_out_0 = rtu_satMax;
  } else if (rtb_UpperRelop_f3) {
    /* Switch: '<S84>/Switch3' */
    rty_out_0 = rtu_satMin;
  } else {
    rty_out_0 = (int16_T)tmp_0;
  }

  /* End of Switch: '<S84>/Switch1' */

  /* Signum: '<S82>/SignDeltaU2' incorporates:
   *  Sum: '<S81>/Sum2'
   */
  if ((int32_T)tmp < 0) {
    tmp_1 = -1;
  } else {
    tmp_1 = ((int32_T)tmp > 0);
  }

  /* Signum: '<S82>/SignDeltaU3' incorporates:
   *  Sum: '<S81>/Sum1'
   */
  if (tmp_0 < 0) {
    tmp_2 = -1;
  } else {
    tmp_2 = (int16_T)(tmp_0 > 0);
  }

  /* Update for UnitDelay: '<S81>/UnitDelay1' incorporates:
   *  Logic: '<S81>/AND1'
   *  Logic: '<S82>/AND1'
   *  RelationalOperator: '<S82>/Equal1'
   *  Signum: '<S82>/SignDeltaU2'
   *  Signum: '<S82>/SignDeltaU3'
   */
  localDW->UnitDelay1_DSTATE = ((tmp_1 == tmp_2) && (rtb_LowerRelop1_ow ||
    rtb_UpperRelop_f3));

  /* Update for Delay: '<S83>/Resettable Delay' incorporates:
   *  Sum: '<S83>/Sum1'
   */
  localDW->icLoad = false;
  localDW->ResettableDelay_DSTATE = rtb_Sum1_d;
  return rty_out_0;
}

/* System initialize for atomic system: '<S88>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

void PI_clamp_fixdtFF_d_Init(DW_PI_clamp_fixdtFF_k *localDW)
{
  /* InitializeConditions for Delay: '<S148>/Resettable Delay' */
  localDW->icLoad = true;
}

#endif

/* System reset for atomic system: '<S88>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

void PI_clamp_fixdtFF_f_Reset(DW_PI_clamp_fixdtFF_k *localDW)
{
  /* InitializeConditions for UnitDelay: '<S146>/UnitDelay1' */
  localDW->UnitDelay1_DSTATE = false;

  /* InitializeConditions for Delay: '<S148>/Resettable Delay' */
  localDW->icLoad = true;
}

#endif

/* Output and update for atomic system: '<S88>/PI_clamp_fixdtFF' */
#if FeedForwardEnable == 1

int16_T PI_clamp_fixdtFF_c(int16_T rtu_err, uint16_T rtu_P, uint16_T rtu_I,
  int16_T rtu_init, int16_T rtu_satMax, int16_T rtu_satMin, int16_T rtu_tgtInput,
  int16_T rtu_FFGain, DW_PI_clamp_fixdtFF_k *localDW)
{
  int16_T rty_out_0;
  int32_T rtb_SignDeltaU2_h;
  int32_T rtb_Sum1_mi;
  int32_T tmp;
  int32_T tmp_0;
  int16_T tmp_1;
  boolean_T rtb_LowerRelop1_ji;
  boolean_T rtb_UpperRelop_c;

  /* Product: '<S146>/Divide2' incorporates:
   *  Signum: '<S147>/SignDeltaU2'
   */
  rtb_SignDeltaU2_h = rtu_err * rtu_I;

  /* Delay: '<S148>/Resettable Delay' */
  if (localDW->icLoad) {
    localDW->ResettableDelay_DSTATE = rtu_init << 16;
  }

  /* Switch: '<S146>/Switch1' incorporates:
   *  Constant: '<S146>/Constant'
   *  Signum: '<S147>/SignDeltaU2'
   *  UnitDelay: '<S146>/UnitDelay1'
   */
  if (localDW->UnitDelay1_DSTATE) {
    tmp_0 = 0;
  } else {
    tmp_0 = rtb_SignDeltaU2_h;
  }

  /* Sum: '<S148>/Sum1' incorporates:
   *  Delay: '<S148>/Resettable Delay'
   *  Switch: '<S146>/Switch1'
   */
  rtb_Sum1_mi = tmp_0 + localDW->ResettableDelay_DSTATE;

  /* Product: '<S146>/Product' */
  tmp_0 = (rtu_FFGain * rtu_tgtInput) >> 12;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* Product: '<S146>/Divide5' */
  tmp = (rtu_err * rtu_P) >> 9;
  if (tmp > 32767) {
    tmp = 32767;
  } else if (tmp < -32768) {
    tmp = -32768;
  }

  /* Sum: '<S146>/Sum1' incorporates:
   *  DataTypeConversion: '<S148>/Data Type Conversion1'
   *  Product: '<S146>/Divide5'
   *  Product: '<S146>/Product'
   *  Sum: '<S148>/Sum1'
   */
  tmp_0 = (((tmp_0 << 1) + tmp) + ((rtb_Sum1_mi >> 16) << 1)) >> 1;
  if (tmp_0 > 32767) {
    tmp_0 = 32767;
  } else if (tmp_0 < -32768) {
    tmp_0 = -32768;
  }

  /* RelationalOperator: '<S149>/LowerRelop1' incorporates:
   *  Sum: '<S146>/Sum1'
   */
  rtb_LowerRelop1_ji = (tmp_0 > rtu_satMax);

  /* RelationalOperator: '<S149>/UpperRelop' incorporates:
   *  Sum: '<S146>/Sum1'
   */
  rtb_UpperRelop_c = (tmp_0 < rtu_satMin);

  /* Switch: '<S149>/Switch1' incorporates:
   *  Sum: '<S146>/Sum1'
   *  Switch: '<S149>/Switch3'
   */
  if (rtb_LowerRelop1_ji) {
    rty_out_0 = rtu_satMax;
  } else if (rtb_UpperRelop_c) {
    /* Switch: '<S149>/Switch3' */
    rty_out_0 = rtu_satMin;
  } else {
    rty_out_0 = (int16_T)tmp_0;
  }

  /* End of Switch: '<S149>/Switch1' */

  /* Signum: '<S147>/SignDeltaU2' */
  if (rtb_SignDeltaU2_h < 0) {
    tmp = -1;
  } else {
    tmp = (rtb_SignDeltaU2_h > 0);
  }

  /* Signum: '<S147>/SignDeltaU3' incorporates:
   *  Sum: '<S146>/Sum1'
   */
  if (tmp_0 < 0) {
    tmp_1 = -1;
  } else {
    tmp_1 = (int16_T)(tmp_0 > 0);
  }

  /* Update for UnitDelay: '<S146>/UnitDelay1' incorporates:
   *  Logic: '<S146>/AND1'
   *  Logic: '<S147>/AND1'
   *  RelationalOperator: '<S147>/Equal1'
   *  Signum: '<S147>/SignDeltaU2'
   *  Signum: '<S147>/SignDeltaU3'
   */
  localDW->UnitDelay1_DSTATE = ((tmp == tmp_1) && (rtb_LowerRelop1_ji ||
    rtb_UpperRelop_c));

  /* Update for Delay: '<S148>/Resettable Delay' incorporates:
   *  Sum: '<S148>/Sum1'
   */
  localDW->icLoad = false;
  localDW->ResettableDelay_DSTATE = rtb_Sum1_mi;
  return rty_out_0;
}

#endif

/* System initialize for function-call system: '<S7>/FOC' */
void FOC_Init(int16_T *rty_Vq, DW_FOC *localDW)
{
  /* Start for If: '<S63>/If1' */
  localDW->If1_ActiveSubsystem = -1;

  /* SystemInitialize for IfAction SubSystem: '<S63>/FOC_Enabled' */
  /* Start for SwitchCase: '<S75>/Switch Case' */
  localDW->SwitchCase_ActiveSubsystem = -1;

  /* Start for If: '<S75>/If1' */
  localDW->If1_ActiveSubsystem_a = -1;

  /* SystemInitialize for IfAction SubSystem: '<S75>/Speed_Mode' */
  /* SystemInitialize for Atomic SubSystem: '<S77>/PI_clamp_fixdt' */
  PI_clamp_fixdt_Init(&localDW->PI_clamp_fixdt_j);

  /* End of SystemInitialize for SubSystem: '<S77>/PI_clamp_fixdt' */
  /* End of SystemInitialize for SubSystem: '<S75>/Speed_Mode' */

  /* SystemInitialize for IfAction SubSystem: '<S75>/Torque_Mode' */
  /* SystemInitialize for Atomic SubSystem: '<S78>/Pi Variant' */
#if FeedForwardEnable == 0

  /* SystemInitialize for Atomic SubSystem: '<S85>/FFoff' */
  /* InitializeConditions for DiscreteIntegrator: '<S127>/Integrator' */
  localDW->Integrator_IC_LOADING = 1U;

  /* End of SystemInitialize for SubSystem: '<S85>/FFoff' */
#elif FeedForwardEnable == 1

  /* SystemInitialize for Atomic SubSystem: '<S85>/FFon' */
  /* SystemInitialize for Atomic SubSystem: '<S88>/PI_clamp_fixdtFF' */
  PI_clamp_fixdtFF_d_Init(&localDW->PI_clamp_fixdtFF_ci);

  /* End of SystemInitialize for SubSystem: '<S88>/PI_clamp_fixdtFF' */
  /* End of SystemInitialize for SubSystem: '<S85>/FFon' */
#endif

  /* End of SystemInitialize for SubSystem: '<S78>/Pi Variant' */
  /* End of SystemInitialize for SubSystem: '<S75>/Torque_Mode' */

  /* SystemInitialize for Merge: '<S75>/Merge' */
  *rty_Vq = 0;

  /* SystemInitialize for IfAction SubSystem: '<S75>/Vd_Calculation' */
  /* SystemInitialize for Atomic SubSystem: '<S79>/Pi Variant' */
#if FeedForwardEnable == 1

  /* SystemInitialize for Atomic SubSystem: '<S150>/FFon' */
  /* SystemInitialize for Atomic SubSystem: '<S153>/PI_clamp_fixdtFF' */
  PI_clamp_fixdtFF_Init(&localDW->PI_clamp_fixdtFF_c2);

  /* End of SystemInitialize for SubSystem: '<S153>/PI_clamp_fixdtFF' */
  /* End of SystemInitialize for SubSystem: '<S150>/FFon' */
#endif

  /* End of SystemInitialize for SubSystem: '<S79>/Pi Variant' */
  /* End of SystemInitialize for SubSystem: '<S75>/Vd_Calculation' */
  /* End of SystemInitialize for SubSystem: '<S63>/FOC_Enabled' */
}

/* Output and update for function-call system: '<S7>/FOC' */
void FOC(uint8_T rtu_z_ctrlTypSel, boolean_T rtu_b_enaAdvCtrl, uint8_T
         rtu_z_ctrlMod, int16_T rtu_r_inpTgtSca, int16_T rtu_r_inpTgtScaAbs,
         int16_T rtu_n_mot, int16_T rtu_id_TgtAbs, int16_T rtu_id, int16_T
         rtu_iq, int16_T rtu_Vd_max, int16_T rtu_Vd_min, int16_T rtu_Vq_max,
         int16_T rtu_Vq_min, int16_T rtu_id_max, int16_T rtu_id_min, int16_T
         rtu_iq_max, int16_T rtu_iq_min, int16_T rtu_r_iqLimProtMax, int16_T
         rtu_r_nLimProtMax, int32_T rtu_iq_limProtErr, int16_T
         rtu_Vq_nLimProtMax, int16_T rtu_VqFinPrev, boolean_T
         rtu_b_cruiseCtrlEna, const P *rtP, int16_T *rty_Vq, int16_T *rty_Vd, DW_FOC *localDW)
{
  /* local block i/o variables */
  int16_T rtb_VariantMergeForOutportVq_Tr;
  int16_T rtb_Divide1_g;
  int16_T rtb_MinMax1_d;
  int8_T rtAction;
  int8_T rtPrevAction;

#if FeedForwardEnable == 0

  int16_T rtb_IProdOut_o;

#endif

  int32_T tmp;
  int16_T rtb_Switch4_idx_0;
  int16_T rtb_Switch4_idx_1;

#if FeedForwardEnable == 0

  int32_T tmp_0;

#endif

  /* If: '<S63>/If1' */
  rtPrevAction = localDW->If1_ActiveSubsystem;
  rtAction = -1;
  if (rtu_z_ctrlTypSel == 2) {
    rtAction = 0;
  }

  localDW->If1_ActiveSubsystem = rtAction;
  if ((rtPrevAction != rtAction) && (rtPrevAction == 0)) {
    /* Disable for SwitchCase: '<S75>/Switch Case' */
    localDW->SwitchCase_ActiveSubsystem = -1;

    /* Disable for If: '<S75>/If1' */
    localDW->If1_ActiveSubsystem_a = -1;
  }

  if (rtAction == 0) {
    /* Outputs for IfAction SubSystem: '<S63>/FOC_Enabled' incorporates:
     *  ActionPort: '<S75>/Action Port'
     */
    /* SwitchCase: '<S75>/Switch Case' incorporates:
     *  Constant: '<S77>/cf_nKi'
     *  Constant: '<S77>/cf_nKp'
     *  Constant: '<S78>/Input'
     *  Constant: '<S78>/cf_iqKi1'
     *  Constant: '<S78>/cf_iqKp1'
     *  Sum: '<S77>/Sum3'
     *  Sum: '<S78>/Sum2'
     */
    rtPrevAction = localDW->SwitchCase_ActiveSubsystem;
    switch (rtu_z_ctrlMod) {
     case 1:
      break;

     case 2:
      rtAction = 1;
      break;

     case 3:
      rtAction = 2;
      break;

     default:
      rtAction = 3;
      break;
    }

    localDW->SwitchCase_ActiveSubsystem = rtAction;
    switch (rtAction) {
     case 0:
      /* Outputs for IfAction SubSystem: '<S75>/Voltage_Mode' incorporates:
       *  ActionPort: '<S80>/Action Port'
       */
      /* MinMax: '<S80>/MinMax' */
      if (rtu_r_inpTgtScaAbs <= rtu_r_iqLimProtMax) {
        rtb_Divide1_g = rtu_r_inpTgtScaAbs;
      } else {
        rtb_Divide1_g = rtu_r_iqLimProtMax;
      }

      if (rtb_Divide1_g > rtu_r_nLimProtMax) {
        rtb_Divide1_g = rtu_r_nLimProtMax;
      }

      /* Signum: '<S80>/SignDeltaU2' */
      if (rtu_r_inpTgtSca < 0) {
        rtb_Switch4_idx_0 = -1;
      } else {
        rtb_Switch4_idx_0 = (int16_T)(rtu_r_inpTgtSca > 0);
      }

      /* Product: '<S80>/Divide1' incorporates:
       *  MinMax: '<S80>/MinMax'
       *  Signum: '<S80>/SignDeltaU2'
       */
      rtb_Divide1_g *= rtb_Switch4_idx_0;

      /* Switch: '<S215>/Switch2' incorporates:
       *  Product: '<S80>/Divide1'
       *  RelationalOperator: '<S215>/LowerRelop1'
       *  RelationalOperator: '<S215>/UpperRelop'
       *  Switch: '<S215>/Switch'
       */
      if (rtb_Divide1_g > rtu_Vq_max) {
        /* SignalConversion: '<S80>/Signal Conversion2' */
        *rty_Vq = rtu_Vq_max;
      } else if (rtb_Divide1_g < rtu_Vq_min) {
        /* Switch: '<S215>/Switch' incorporates:
         *  SignalConversion: '<S80>/Signal Conversion2'
         */
        *rty_Vq = rtu_Vq_min;
      } else {
        /* SignalConversion: '<S80>/Signal Conversion2' incorporates:
         *  Switch: '<S215>/Switch'
         */
        *rty_Vq = rtb_Divide1_g;
      }

      /* End of Switch: '<S215>/Switch2' */
      /* End of Outputs for SubSystem: '<S75>/Voltage_Mode' */
      break;

     case 1:
      if (rtAction != rtPrevAction) {
        /* SystemReset for IfAction SubSystem: '<S75>/Speed_Mode' incorporates:
         *  ActionPort: '<S77>/Action Port'
         */
        /* SystemReset for Atomic SubSystem: '<S77>/PI_clamp_fixdt' */
        /* SystemReset for SwitchCase: '<S75>/Switch Case' */
        PI_clamp_fixdt_Reset(&localDW->PI_clamp_fixdt_j);

        /* End of SystemReset for SubSystem: '<S77>/PI_clamp_fixdt' */
        /* End of SystemReset for SubSystem: '<S75>/Speed_Mode' */
      }

      /* Outputs for IfAction SubSystem: '<S75>/Speed_Mode' incorporates:
       *  ActionPort: '<S77>/Action Port'
       */
      /* DataTypeConversion: '<S77>/Data Type Conversion2' incorporates:
       *  Constant: '<S77>/n_cruiseMotTgt'
       */
      rtb_Divide1_g = (int16_T)(rtP->n_cruiseMotTgt << 4);

      /* Switch: '<S77>/Switch4' incorporates:
       *  DataTypeConversion: '<S77>/Data Type Conversion2'
       *  Logic: '<S77>/Logical Operator1'
       *  RelationalOperator: '<S77>/Relational Operator3'
       */
      if (rtu_b_cruiseCtrlEna && (rtb_Divide1_g != 0)) {
        /* Switch: '<S77>/Switch3' incorporates:
         *  MinMax: '<S77>/MinMax4'
         */
        if (rtb_Divide1_g > 0) {
          /* Switch: '<S77>/Switch4' */
          rtb_Switch4_idx_0 = rtu_Vq_max;

          /* MinMax: '<S77>/MinMax3' */
          if (rtu_r_inpTgtSca >= rtu_Vq_min) {
            /* Switch: '<S77>/Switch4' */
            rtb_Switch4_idx_1 = rtu_r_inpTgtSca;
          } else {
            /* Switch: '<S77>/Switch4' */
            rtb_Switch4_idx_1 = rtu_Vq_min;
          }

          /* End of MinMax: '<S77>/MinMax3' */
        } else {
          if (rtu_Vq_max <= rtu_r_inpTgtSca) {
            /* MinMax: '<S77>/MinMax4' incorporates:
             *  Switch: '<S77>/Switch4'
             */
            rtb_Switch4_idx_0 = rtu_Vq_max;
          } else {
            /* Switch: '<S77>/Switch4' incorporates:
             *  MinMax: '<S77>/MinMax4'
             */
            rtb_Switch4_idx_0 = rtu_r_inpTgtSca;
          }

          /* Switch: '<S77>/Switch4' */
          rtb_Switch4_idx_1 = rtu_Vq_min;
        }

        /* End of Switch: '<S77>/Switch3' */
      } else {
        rtb_Switch4_idx_0 = rtu_Vq_max;
        rtb_Switch4_idx_1 = rtu_Vq_min;
      }

      /* End of Switch: '<S77>/Switch4' */

      /* Switch: '<S77>/Switch2' */
      if (!rtu_b_cruiseCtrlEna) {
        rtb_Divide1_g = rtu_r_inpTgtSca;
      }

      /* Sum: '<S77>/Sum3' incorporates:
       *  Switch: '<S77>/Switch2'
       */
      tmp = rtb_Divide1_g - rtu_n_mot;
      if (tmp > 32767) {
        tmp = 32767;
      } else if (tmp < -32768) {
        tmp = -32768;
      }

      /* Outputs for Atomic SubSystem: '<S77>/PI_clamp_fixdt' */
      *rty_Vq = PI_clamp_fixdt((int16_T)tmp, rtP->cf_nKp, rtP->cf_nKi,
        rtu_VqFinPrev, rtb_Switch4_idx_0, rtb_Switch4_idx_1, rtu_iq_limProtErr,
        &localDW->PI_clamp_fixdt_j);

      /* End of Outputs for SubSystem: '<S77>/PI_clamp_fixdt' */
      /* End of Outputs for SubSystem: '<S75>/Speed_Mode' */
      break;

     case 2:
      if (rtAction != rtPrevAction) {
        /* SystemReset for IfAction SubSystem: '<S75>/Torque_Mode' incorporates:
         *  ActionPort: '<S78>/Action Port'
         */
        /* SystemReset for Atomic SubSystem: '<S78>/Pi Variant' */
        /* SystemReset for SwitchCase: '<S75>/Switch Case' */
#if FeedForwardEnable == 0

        /* SystemReset for Atomic SubSystem: '<S85>/FFoff' */
        /* InitializeConditions for DiscreteIntegrator: '<S127>/Integrator' */
        localDW->Integrator_IC_LOADING = 1U;

        /* End of SystemReset for SubSystem: '<S85>/FFoff' */
#elif FeedForwardEnable == 1

        /* SystemReset for Atomic SubSystem: '<S85>/FFon' */
        /* SystemReset for Atomic SubSystem: '<S88>/PI_clamp_fixdtFF' */
        PI_clamp_fixdtFF_f_Reset(&localDW->PI_clamp_fixdtFF_ci);

        /* End of SystemReset for SubSystem: '<S88>/PI_clamp_fixdtFF' */
        /* End of SystemReset for SubSystem: '<S85>/FFon' */
#endif

        /* End of SystemReset for SubSystem: '<S78>/Pi Variant' */
        /* End of SystemReset for SubSystem: '<S75>/Torque_Mode' */
      }

      /* Outputs for IfAction SubSystem: '<S75>/Torque_Mode' incorporates:
       *  ActionPort: '<S78>/Action Port'
       */
      /* Gain: '<S78>/Gain4' */
      rtb_Divide1_g = (int16_T)-rtu_Vq_nLimProtMax;

      /* MinMax: '<S78>/MinMax2' incorporates:
       *  Gain: '<S78>/Gain4'
       */
      if ((int16_T)-rtu_Vq_nLimProtMax < rtu_Vq_min) {
        rtb_Divide1_g = rtu_Vq_min;
      }

      /* End of MinMax: '<S78>/MinMax2' */

      /* MinMax: '<S78>/MinMax1' */
      if (rtu_Vq_max <= rtu_Vq_nLimProtMax) {
        rtb_MinMax1_d = rtu_Vq_max;
      } else {
        rtb_MinMax1_d = rtu_Vq_nLimProtMax;
      }

      /* End of MinMax: '<S78>/MinMax1' */

      /* Switch: '<S86>/Switch2' incorporates:
       *  RelationalOperator: '<S86>/LowerRelop1'
       *  RelationalOperator: '<S86>/UpperRelop'
       *  Switch: '<S86>/Switch'
       */
      if (rtu_r_inpTgtSca > rtu_iq_max) {
        rtb_Switch4_idx_1 = rtu_iq_max;
      } else if (rtu_r_inpTgtSca < rtu_iq_min) {
        /* Switch: '<S86>/Switch' incorporates:
         *  Switch: '<S86>/Switch2'
         */
        rtb_Switch4_idx_1 = rtu_iq_min;
      } else {
        rtb_Switch4_idx_1 = rtu_r_inpTgtSca;
      }

      /* End of Switch: '<S86>/Switch2' */

      /* Sum: '<S78>/Sum2' incorporates:
       *  Switch: '<S86>/Switch2'
       */
      tmp = rtb_Switch4_idx_1 - rtu_iq;
      if (tmp > 32767) {
        tmp = 32767;
      } else if (tmp < -32768) {
        tmp = -32768;
      }

      /* Outputs for Atomic SubSystem: '<S78>/Pi Variant' */
#if FeedForwardEnable == 0

      /* Outputs for Atomic SubSystem: '<S85>/FFoff' */
      /* Product: '<S124>/IProd Out' incorporates:
       *  Constant: '<S78>/cf_iqKi1'
       *  Sum: '<S78>/Sum2'
       */
      rtb_IProdOut_o = (int16_T)((tmp * rtP->cf_iqKi) >> 16);

      /* DiscreteIntegrator: '<S127>/Integrator' */
      if (localDW->Integrator_IC_LOADING != 0) {
        localDW->Integrator_DSTATE = rtu_VqFinPrev;
      }

      /* Product: '<S132>/PProd Out' incorporates:
       *  Constant: '<S78>/cf_iqKp1'
       *  Sum: '<S78>/Sum2'
       */
      tmp_0 = (tmp * rtP->cf_iqKp) >> 10;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      /* Sum: '<S137>/Sum' incorporates:
       *  DiscreteIntegrator: '<S127>/Integrator'
       *  Product: '<S132>/PProd Out'
       */
      tmp_0 += localDW->Integrator_DSTATE;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      /* Switch: '<S119>/Switch' incorporates:
       *  MinMax: '<S78>/MinMax1'
       *  MinMax: '<S78>/MinMax2'
       *  RelationalOperator: '<S119>/u_GTE_up'
       *  RelationalOperator: '<S119>/u_GT_lo'
       *  Sum: '<S137>/Sum'
       *  Switch: '<S119>/Switch1'
       */
      if (tmp_0 >= rtb_MinMax1_d) {
        rtb_Switch4_idx_0 = rtb_MinMax1_d;
      } else if (tmp_0 > rtb_Divide1_g) {
        /* Switch: '<S119>/Switch1' */
        rtb_Switch4_idx_0 = (int16_T)tmp_0;
      } else {
        rtb_Switch4_idx_0 = rtb_Divide1_g;
      }

      /* Sum: '<S119>/Diff' incorporates:
       *  Sum: '<S137>/Sum'
       *  Switch: '<S119>/Switch'
       */
      rtb_Switch4_idx_0 = (int16_T)(tmp_0 - rtb_Switch4_idx_0);

      /* Switch: '<S135>/Switch2' incorporates:
       *  MinMax: '<S78>/MinMax1'
       *  MinMax: '<S78>/MinMax2'
       *  RelationalOperator: '<S135>/LowerRelop1'
       *  RelationalOperator: '<S135>/UpperRelop'
       *  Sum: '<S137>/Sum'
       *  Switch: '<S135>/Switch'
       */
      if (tmp_0 > rtb_MinMax1_d) {
        /* VariantMerge generated from: '<S85>/Vq_Trq' */
        rtb_VariantMergeForOutportVq_Tr = rtb_MinMax1_d;
      } else if (tmp_0 < rtb_Divide1_g) {
        /* Switch: '<S135>/Switch' incorporates:
         *  MinMax: '<S78>/MinMax2'
         *  VariantMerge generated from: '<S85>/Vq_Trq'
         */
        rtb_VariantMergeForOutportVq_Tr = rtb_Divide1_g;
      } else {
        /* VariantMerge generated from: '<S85>/Vq_Trq' */
        rtb_VariantMergeForOutportVq_Tr = (int16_T)tmp_0;
      }

      /* End of Switch: '<S135>/Switch2' */

      /* Update for DiscreteIntegrator: '<S127>/Integrator' */
      localDW->Integrator_IC_LOADING = 0U;

      /* Switch: '<S116>/Switch1' incorporates:
       *  Constant: '<S116>/Constant'
       *  Constant: '<S116>/Constant2'
       *  RelationalOperator: '<S116>/fix for DT propagation issue'
       *  Sum: '<S119>/Diff'
       */
      if (rtb_Switch4_idx_0 > 0) {
        rtPrevAction = 1;
      } else {
        rtPrevAction = -1;
      }

      /* Switch: '<S116>/Switch2' incorporates:
       *  Constant: '<S116>/Constant3'
       *  Constant: '<S116>/Constant4'
       *  Product: '<S124>/IProd Out'
       *  RelationalOperator: '<S116>/fix for DT propagation issue1'
       */
      if (rtb_IProdOut_o > 0) {
        rtAction = 1;
      } else {
        rtAction = -1;
      }

      /* Switch: '<S116>/Switch' incorporates:
       *  Constant: '<S116>/Constant1'
       *  Logic: '<S116>/AND3'
       *  RelationalOperator: '<S116>/Equal1'
       *  RelationalOperator: '<S116>/Relational Operator'
       *  Sum: '<S119>/Diff'
       *  Switch: '<S116>/Switch1'
       *  Switch: '<S116>/Switch2'
       */
      if ((rtb_Switch4_idx_0 != 0) && (rtPrevAction == rtAction)) {
        rtb_IProdOut_o = 0;
      }

      /* Update for DiscreteIntegrator: '<S127>/Integrator' incorporates:
       *  Switch: '<S116>/Switch'
       */
      tmp_0 = localDW->Integrator_DSTATE + rtb_IProdOut_o;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      localDW->Integrator_DSTATE = (int16_T)tmp_0;

      /* End of Outputs for SubSystem: '<S85>/FFoff' */
#elif FeedForwardEnable == 1

      /* Outputs for Atomic SubSystem: '<S85>/FFon' */
      /* Outputs for Atomic SubSystem: '<S88>/PI_clamp_fixdtFF' */
      rtb_VariantMergeForOutportVq_Tr = PI_clamp_fixdtFF_c((int16_T)tmp,
        rtP->cf_iqKp, rtP->cf_iqKi, rtu_VqFinPrev, rtb_MinMax1_d, rtb_Divide1_g,
        rtb_Switch4_idx_1, rtP->ff_gain, &localDW->PI_clamp_fixdtFF_ci);

      /* End of Outputs for SubSystem: '<S88>/PI_clamp_fixdtFF' */
      /* End of Outputs for SubSystem: '<S85>/FFon' */
#endif

      /* End of Outputs for SubSystem: '<S78>/Pi Variant' */

      /* SignalConversion generated from: '<S78>/Vq_Trq' incorporates:
       *  Constant: '<S78>/Input'
       *  Constant: '<S78>/cf_iqKi1'
       *  Constant: '<S78>/cf_iqKp1'
       *  Sum: '<S78>/Sum2'
       *  VariantMerge generated from: '<S85>/Vq_Trq'
       */
      *rty_Vq = rtb_VariantMergeForOutportVq_Tr;

      /* End of Outputs for SubSystem: '<S75>/Torque_Mode' */
      break;

     default:
      /* Outputs for IfAction SubSystem: '<S75>/Open_Mode' incorporates:
       *  ActionPort: '<S76>/Action Port'
       */
      /* SignalConversion generated from: '<S76>/r_inpTgtSca' */
      *rty_Vq = rtu_r_inpTgtSca;

      /* End of Outputs for SubSystem: '<S75>/Open_Mode' */
      break;
    }

    /* End of SwitchCase: '<S75>/Switch Case' */

    /* If: '<S75>/If1' incorporates:
     *  Constant: '<S79>/Input'
     *  Constant: '<S79>/cf_idKi2'
     *  Constant: '<S79>/cf_idKp2'
     *  Constant: '<S79>/constant2'
     *  Sum: '<S79>/Sum3'
     */
    rtPrevAction = localDW->If1_ActiveSubsystem_a;
    rtAction = -1;
    if (rtu_b_enaAdvCtrl) {
      rtAction = 0;
    }

    localDW->If1_ActiveSubsystem_a = rtAction;
    if (rtAction == 0) {
      if (rtPrevAction != 0) {
        /* SystemReset for IfAction SubSystem: '<S75>/Vd_Calculation' incorporates:
         *  ActionPort: '<S79>/Action Port'
         */
        /* SystemReset for Atomic SubSystem: '<S79>/Pi Variant' */
        /* SystemReset for If: '<S75>/If1' */
#if FeedForwardEnable == 0

        /* SystemReset for Atomic SubSystem: '<S150>/FFoff' */
        /* InitializeConditions for DiscreteIntegrator: '<S192>/Integrator' incorporates:
         *  Constant: '<S79>/constant2'
         */
        localDW->Integrator_DSTATE_a = localDW->constant2;

        /* End of SystemReset for SubSystem: '<S150>/FFoff' */
#elif FeedForwardEnable == 1

        /* SystemReset for Atomic SubSystem: '<S150>/FFon' */
        /* SystemReset for Atomic SubSystem: '<S153>/PI_clamp_fixdtFF' */
        PI_clamp_fixdtFF_Reset(&localDW->PI_clamp_fixdtFF_c2);

        /* End of SystemReset for SubSystem: '<S153>/PI_clamp_fixdtFF' */
        /* End of SystemReset for SubSystem: '<S150>/FFon' */
#endif

        /* End of SystemReset for If: '<S75>/If1' */
        /* End of SystemReset for SubSystem: '<S79>/Pi Variant' */
        /* End of SystemReset for SubSystem: '<S75>/Vd_Calculation' */
      }

      /* Outputs for IfAction SubSystem: '<S75>/Vd_Calculation' incorporates:
       *  ActionPort: '<S79>/Action Port'
       */
      /* Gain: '<S79>/toNegative' */
      rtb_Divide1_g = (int16_T)-rtu_id_TgtAbs;

      /* Switch: '<S151>/Switch2' incorporates:
       *  Gain: '<S79>/toNegative'
       *  RelationalOperator: '<S151>/LowerRelop1'
       *  RelationalOperator: '<S151>/UpperRelop'
       *  Switch: '<S151>/Switch'
       */
      if ((int16_T)-rtu_id_TgtAbs > rtu_id_max) {
        rtb_Divide1_g = rtu_id_max;
      } else if ((int16_T)-rtu_id_TgtAbs < rtu_id_min) {
        /* Switch: '<S151>/Switch' incorporates:
         *  Switch: '<S151>/Switch2'
         */
        rtb_Divide1_g = rtu_id_min;
      }

      /* End of Switch: '<S151>/Switch2' */

      /* Sum: '<S79>/Sum3' incorporates:
       *  Switch: '<S151>/Switch2'
       */
      tmp = rtb_Divide1_g - rtu_id;
      if (tmp > 32767) {
        tmp = 32767;
      } else if (tmp < -32768) {
        tmp = -32768;
      }

      localDW->constant2 = 0;

      /* Outputs for Atomic SubSystem: '<S79>/Pi Variant' */
#if FeedForwardEnable == 0

      /* Outputs for Atomic SubSystem: '<S150>/FFoff' */
      /* Product: '<S189>/IProd Out' incorporates:
       *  Constant: '<S79>/cf_idKi2'
       *  Sum: '<S79>/Sum3'
       */
      rtb_Switch4_idx_1 = (int16_T)((tmp * rtP->cf_idKi) >> 16);

      /* Product: '<S197>/PProd Out' incorporates:
       *  Constant: '<S79>/cf_idKp2'
       *  Sum: '<S79>/Sum3'
       */
      tmp_0 = (tmp * rtP->cf_idKp) >> 10;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      /* Sum: '<S202>/Sum' incorporates:
       *  DiscreteIntegrator: '<S192>/Integrator'
       *  Product: '<S197>/PProd Out'
       */
      tmp_0 += localDW->Integrator_DSTATE_a;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      /* Switch: '<S184>/Switch' incorporates:
       *  RelationalOperator: '<S184>/u_GTE_up'
       *  RelationalOperator: '<S184>/u_GT_lo'
       *  Sum: '<S202>/Sum'
       *  Switch: '<S184>/Switch1'
       */
      if (tmp_0 >= rtu_Vd_max) {
        rtb_Switch4_idx_0 = rtu_Vd_max;
      } else if (tmp_0 > rtu_Vd_min) {
        /* Switch: '<S184>/Switch1' */
        rtb_Switch4_idx_0 = (int16_T)tmp_0;
      } else {
        rtb_Switch4_idx_0 = rtu_Vd_min;
      }

      /* Sum: '<S184>/Diff' incorporates:
       *  Sum: '<S202>/Sum'
       *  Switch: '<S184>/Switch'
       */
      rtb_IProdOut_o = (int16_T)(tmp_0 - rtb_Switch4_idx_0);

      /* Switch: '<S200>/Switch2' incorporates:
       *  RelationalOperator: '<S200>/LowerRelop1'
       *  RelationalOperator: '<S200>/UpperRelop'
       *  Sum: '<S202>/Sum'
       *  Switch: '<S200>/Switch'
       */
      if (tmp_0 > rtu_Vd_max) {
        *rty_Vd = rtu_Vd_max;
      } else if (tmp_0 < rtu_Vd_min) {
        /* Switch: '<S200>/Switch' */
        *rty_Vd = rtu_Vd_min;
      } else {
        *rty_Vd = (int16_T)tmp_0;
      }

      /* End of Switch: '<S200>/Switch2' */

      /* Switch: '<S181>/Switch1' incorporates:
       *  Constant: '<S181>/Constant'
       *  Constant: '<S181>/Constant2'
       *  RelationalOperator: '<S181>/fix for DT propagation issue'
       *  Sum: '<S184>/Diff'
       */
      if (rtb_IProdOut_o > 0) {
        rtPrevAction = 1;
      } else {
        rtPrevAction = -1;
      }

      /* Switch: '<S181>/Switch2' incorporates:
       *  Constant: '<S181>/Constant3'
       *  Constant: '<S181>/Constant4'
       *  Product: '<S189>/IProd Out'
       *  RelationalOperator: '<S181>/fix for DT propagation issue1'
       */
      if (rtb_Switch4_idx_1 > 0) {
        rtAction = 1;
      } else {
        rtAction = -1;
      }

      /* Switch: '<S181>/Switch' incorporates:
       *  Constant: '<S181>/Constant1'
       *  Logic: '<S181>/AND3'
       *  RelationalOperator: '<S181>/Equal1'
       *  RelationalOperator: '<S181>/Relational Operator'
       *  Sum: '<S184>/Diff'
       *  Switch: '<S181>/Switch1'
       *  Switch: '<S181>/Switch2'
       */
      if ((rtb_IProdOut_o != 0) && (rtPrevAction == rtAction)) {
        rtb_Switch4_idx_1 = 0;
      }

      /* Update for DiscreteIntegrator: '<S192>/Integrator' incorporates:
       *  Switch: '<S181>/Switch'
       */
      tmp_0 = localDW->Integrator_DSTATE_a + rtb_Switch4_idx_1;
      if (tmp_0 > 32767) {
        tmp_0 = 32767;
      } else if (tmp_0 < -32768) {
        tmp_0 = -32768;
      }

      localDW->Integrator_DSTATE_a = (int16_T)tmp_0;

      /* End of Update for DiscreteIntegrator: '<S192>/Integrator' */
      /* End of Outputs for SubSystem: '<S150>/FFoff' */
#elif FeedForwardEnable == 1

      /* Outputs for Atomic SubSystem: '<S150>/FFon' */
      /* Outputs for Atomic SubSystem: '<S153>/PI_clamp_fixdtFF' */
#if FeedForwardEnable != 0 && FeedForwardEnable != 1

      UNUSED_PARAMETER(rtu_Vd_max);

#endif

#if FeedForwardEnable != 0 && FeedForwardEnable != 1

      UNUSED_PARAMETER(rtu_Vd_min);

#endif

#if FeedForwardEnable != 0 && FeedForwardEnable != 1

      UNUSED_PARAMETER(rty_Vd);

#endif

      *rty_Vd = PI_clamp_fixdtFF((int16_T)tmp, rtP->cf_idKp, rtP->cf_idKi, 0,
        rtu_Vd_max, rtu_Vd_min, rtb_Divide1_g, rtP->ff_gain,
        &localDW->PI_clamp_fixdtFF_c2);

      /* End of Outputs for SubSystem: '<S153>/PI_clamp_fixdtFF' */
      /* End of Outputs for SubSystem: '<S150>/FFon' */
#endif

      /* End of Outputs for SubSystem: '<S79>/Pi Variant' */
      /* End of Outputs for SubSystem: '<S75>/Vd_Calculation' */
    }

    /* End of If: '<S75>/If1' */
    /* End of Outputs for SubSystem: '<S63>/FOC_Enabled' */
  }

  /* End of If: '<S63>/If1' */
}

/* Model step function */
void BLDC_controller_step(RT_MODEL *const rtM)
{
  P *rtP = ((P *) rtM->defaultParam);
  DW *rtDW = rtM->dwork;
  ExtU *rtU = (ExtU *) rtM->inputs;
  ExtY *rtY = (ExtY *) rtM->outputs;
  int16_T Abs5;
  int16_T Switch2;
  int16_T rtb_Switch2_j;
  uint16_T rtb_Divide1;
  uint16_T rtb_f;
  int8_T UnitDelay3;
  int8_T rtb_Sum2_ii;
  uint8_T Sum;
  uint8_T rtb_k;
  boolean_T rtb_LogicalOperator;
  boolean_T rtb_RelationalOperator;
  boolean_T rtb_RelationalOperator4;
  boolean_T rtb_n_commDeacv;

  /* Outputs for Atomic SubSystem: '<Root>/BLDC_controller' */
  /* Sum: '<S11>/Sum' incorporates:
   *  Gain: '<S11>/g_Ha'
   *  Gain: '<S11>/g_Hb'
   */
  Sum = (uint8_T)((uint8_T)((uint8_T)(rtU->b_hallA << 2) + (uint8_T)
    (rtU->b_hallB << 1)) + rtU->b_hallC);

  /* Logic: '<S10>/Logical Operator' incorporates:
   *  UnitDelay: '<S10>/UnitDelay1'
   *  UnitDelay: '<S10>/UnitDelay2'
   *  UnitDelay: '<S10>/UnitDelay3'
   */
  rtb_LogicalOperator = (boolean_T)((rtU->b_hallA != 0) ^ (rtU->b_hallB != 0) ^
    (rtU->b_hallC != 0) ^ (rtDW->UnitDelay3_DSTATE_f != 0) ^
    (rtDW->UnitDelay1_DSTATE != 0)) ^ (rtDW->UnitDelay2_DSTATE_k != 0);

  /* If: '<S13>/If2' incorporates:
   *  If: '<S3>/If2'
   */
  if (rtb_LogicalOperator) {
    /* Outputs for IfAction SubSystem: '<S3>/F01_03_Direction_Detection' incorporates:
     *  ActionPort: '<S12>/Action Port'
     */
    /* UnitDelay: '<S12>/UnitDelay3' */
    UnitDelay3 = rtDW->Switch2_a;

    /* Sum: '<S12>/Sum2' incorporates:
     *  Constant: '<S11>/vec_hallToPos'
     *  Selector: '<S11>/Selector'
     *  UnitDelay: '<S12>/UnitDelay2'
     */
    rtb_Sum2_ii = (int8_T)(rtConstP.vec_hallToPos_Value[Sum] -
      rtDW->UnitDelay2_DSTATE_o);

    /* Switch: '<S12>/Switch2' incorporates:
     *  Constant: '<S12>/Constant20'
     *  Constant: '<S12>/Constant8'
     *  Logic: '<S12>/Logical Operator3'
     *  RelationalOperator: '<S12>/Relational Operator1'
     *  RelationalOperator: '<S12>/Relational Operator6'
     */
    if ((rtb_Sum2_ii == 1) || (rtb_Sum2_ii == -5)) {
      /* Switch: '<S12>/Switch2' incorporates:
       *  Constant: '<S12>/Constant24'
       */
      rtDW->Switch2_a = 1;
    } else {
      /* Switch: '<S12>/Switch2' incorporates:
       *  Constant: '<S12>/Constant23'
       */
      rtDW->Switch2_a = -1;
    }

    /* End of Switch: '<S12>/Switch2' */

    /* Update for UnitDelay: '<S12>/UnitDelay2' incorporates:
     *  Constant: '<S11>/vec_hallToPos'
     *  Selector: '<S11>/Selector'
     */
    rtDW->UnitDelay2_DSTATE_o = rtConstP.vec_hallToPos_Value[Sum];

    /* End of Outputs for SubSystem: '<S3>/F01_03_Direction_Detection' */

    /* Outputs for IfAction SubSystem: '<S13>/Raw_Motor_Speed_Estimation' incorporates:
     *  ActionPort: '<S17>/Action Port'
     */
    /* SignalConversion generated from: '<S17>/z_counterRawPrev' incorporates:
     *  UnitDelay: '<S13>/UnitDelay3'
     */
    rtDW->z_counterRawPrev = rtDW->UnitDelay3_DSTATE;

    /* Sum: '<S17>/Sum7' incorporates:
     *  UnitDelay: '<S17>/UnitDelay4'
     */
    rtDW->Switch3 = (int16_T)(rtDW->z_counterRawPrev - rtDW->UnitDelay4_DSTATE);

    /* Abs: '<S17>/Abs2' */
    if (rtDW->Switch3 < 0) {
      rtDW->Switch3 = (int16_T)-rtDW->Switch3;
    }

    /* End of Abs: '<S17>/Abs2' */

    /* Relay: '<S17>/dz_cntTrnsDet' */
    rtDW->dz_cntTrnsDet_Mode = ((rtDW->Switch3 >= rtP->dz_cntTrnsDetHi) ||
      ((rtDW->Switch3 > rtP->dz_cntTrnsDetLo) && rtDW->dz_cntTrnsDet_Mode));

    /* Relay: '<S17>/dz_cntTrnsDet' */
    rtDW->dz_cntTrnsDet = rtDW->dz_cntTrnsDet_Mode;

    /* RelationalOperator: '<S17>/Relational Operator4' */
    rtb_RelationalOperator4 = (rtDW->Switch2_a != UnitDelay3);

    /* Switch: '<S17>/Switch3' incorporates:
     *  Constant: '<S17>/Constant4'
     *  Logic: '<S17>/Logical Operator1'
     *  Switch: '<S17>/Switch1'
     *  Switch: '<S17>/Switch2'
     *  UnitDelay: '<S17>/UnitDelay1'
     */
    if (rtb_RelationalOperator4 && rtDW->UnitDelay1_DSTATE_g) {
      rtDW->Switch3 = 0;
    } else if (rtb_RelationalOperator4) {
      /* Switch: '<S17>/Switch3' incorporates:
       *  Switch: '<S17>/Switch2'
       *  UnitDelay: '<S13>/UnitDelay4'
       */
      rtDW->Switch3 = rtDW->UnitDelay4_DSTATE_f;
    } else if (rtDW->dz_cntTrnsDet) {
      /* Switch: '<S17>/Switch3' incorporates:
       *  Constant: '<S17>/cf_speedCoef'
       *  Product: '<S17>/Divide14'
       *  Switch: '<S17>/Switch1'
       *  Switch: '<S17>/Switch2'
       */
      rtDW->Switch3 = (int16_T)((rtP->cf_speedCoef << 4) / rtDW->z_counterRawPrev);
    } else {
      /* Switch: '<S17>/Switch3' incorporates:
       *  Constant: '<S17>/cf_speedCoef'
       *  Product: '<S17>/Divide13'
       *  Sum: '<S17>/Sum13'
       *  Switch: '<S17>/Switch1'
       *  Switch: '<S17>/Switch2'
       *  UnitDelay: '<S17>/UnitDelay2'
       *  UnitDelay: '<S17>/UnitDelay3'
       *  UnitDelay: '<S17>/UnitDelay5'
       */
      rtDW->Switch3 = (int16_T)((rtP->cf_speedCoef << 4) / (int16_T)((int16_T)
        ((int16_T)(rtDW->UnitDelay2_DSTATE + rtDW->UnitDelay3_DSTATE_k) +
         rtDW->UnitDelay5_DSTATE) + rtDW->z_counterRawPrev));
    }

    /* End of Switch: '<S17>/Switch3' */

    /* Product: '<S17>/Divide11' incorporates:
     *  Switch: '<S17>/Switch3'
     */
    rtDW->Divide11 = (int16_T)(rtDW->Switch3 * rtDW->Switch2_a);

    /* Update for UnitDelay: '<S17>/UnitDelay4' */
    rtDW->UnitDelay4_DSTATE = rtDW->z_counterRawPrev;

    /* Update for UnitDelay: '<S17>/UnitDelay2' incorporates:
     *  UnitDelay: '<S17>/UnitDelay3'
     */
    rtDW->UnitDelay2_DSTATE = rtDW->UnitDelay3_DSTATE_k;

    /* Update for UnitDelay: '<S17>/UnitDelay3' incorporates:
     *  UnitDelay: '<S17>/UnitDelay5'
     */
    rtDW->UnitDelay3_DSTATE_k = rtDW->UnitDelay5_DSTATE;

    /* Update for UnitDelay: '<S17>/UnitDelay5' */
    rtDW->UnitDelay5_DSTATE = rtDW->z_counterRawPrev;

    /* Update for UnitDelay: '<S17>/UnitDelay1' */
    rtDW->UnitDelay1_DSTATE_g = rtb_RelationalOperator4;

    /* End of Outputs for SubSystem: '<S13>/Raw_Motor_Speed_Estimation' */
  }

  /* End of If: '<S13>/If2' */

  /* Outputs for Atomic SubSystem: '<S13>/Counter' */
  /* Constant: '<S13>/Constant6' incorporates:
   *  Constant: '<S13>/z_maxCntRst2'
   */
  rtDW->Switch3 = Counter(1, rtP->z_maxCntRst, rtb_LogicalOperator,
    &rtDW->Counter_c);

  /* End of Outputs for SubSystem: '<S13>/Counter' */

  /* Switch: '<S13>/Switch2' incorporates:
   *  Constant: '<S13>/z_maxCntRst'
   *  RelationalOperator: '<S13>/Relational Operator2'
   */
  if (rtDW->Switch3 > rtP->z_maxCntRst) {
    /* Switch: '<S13>/Switch2' incorporates:
     *  Constant: '<S13>/Constant4'
     */
    Switch2 = 0;
  } else {
    /* Switch: '<S13>/Switch2' incorporates:
     *  Product: '<S17>/Divide11'
     */
    Switch2 = rtDW->Divide11;
  }

  /* End of Switch: '<S13>/Switch2' */

  /* Abs: '<S13>/Abs5' incorporates:
   *  Switch: '<S13>/Switch2'
   */
  if (Switch2 < 0) {
    /* Abs: '<S13>/Abs5' */
    Abs5 = (int16_T)-Switch2;
  } else {
    /* Abs: '<S13>/Abs5' */
    Abs5 = Switch2;
  }

  /* End of Abs: '<S13>/Abs5' */

  /* Relay: '<S13>/n_commDeacv' incorporates:
   *  Abs: '<S13>/Abs5'
   */
  rtDW->n_commDeacv_Mode = ((Abs5 >= rtP->n_commDeacvHi) || ((Abs5 >
    rtP->n_commAcvLo) && rtDW->n_commDeacv_Mode));

  /* Logic: '<S13>/Logical Operator3' incorporates:
   *  Constant: '<S13>/b_angleMeasEna'
   *  Logic: '<S13>/Logical Operator1'
   *  Logic: '<S13>/Logical Operator2'
   *  Relay: '<S13>/n_commDeacv'
   */
  rtb_LogicalOperator = (rtP->b_angleMeasEna || (rtDW->n_commDeacv_Mode &&
    (!rtDW->dz_cntTrnsDet)));

  /* UnitDelay: '<S2>/UnitDelay2' */
  rtb_RelationalOperator4 = rtDW->UnitDelay2_DSTATE_g;

  /* UnitDelay: '<S2>/UnitDelay5' */
  rtb_n_commDeacv = rtDW->UnitDelay5_DSTATE_l;

  /* Saturate: '<S1>/Saturation' */
  rtDW->sigIdx = rtU->i_phaAB << 4;
  if (rtDW->sigIdx >= 27200) {
    rtDW->Saturation = 27200;
  } else if (rtDW->sigIdx <= -27200) {
    rtDW->Saturation = -27200;
  } else {
    rtDW->Saturation = (int16_T)(rtU->i_phaAB << 4);
  }

  /* End of Saturate: '<S1>/Saturation' */

  /* Saturate: '<S1>/Saturation1' */
  rtDW->sigIdx = rtU->i_phaBC << 4;
  if (rtDW->sigIdx >= 27200) {
    rtDW->Saturation1 = 27200;
  } else if (rtDW->sigIdx <= -27200) {
    rtDW->Saturation1 = -27200;
  } else {
    rtDW->Saturation1 = (int16_T)(rtU->i_phaBC << 4);
  }

  /* End of Saturate: '<S1>/Saturation1' */

  /* If: '<S3>/If1' incorporates:
   *  Constant: '<S3>/b_angleMeasEna'
   */
  if (!rtP->b_angleMeasEna) {
    /* Outputs for IfAction SubSystem: '<S3>/F01_05_Electrical_Angle_Estimation' incorporates:
     *  ActionPort: '<S14>/Action Port'
     */
    /* Switch: '<S14>/Switch3' incorporates:
     *  Constant: '<S11>/vec_hallToPos'
     *  Constant: '<S14>/Constant16'
     *  Constant: '<S14>/Constant2'
     *  RelationalOperator: '<S14>/Relational Operator7'
     *  Selector: '<S11>/Selector'
     *  Sum: '<S14>/Sum1'
     */
    if (rtDW->Switch2_a == 1) {
      rtb_Sum2_ii = rtConstP.vec_hallToPos_Value[Sum];
    } else {
      rtb_Sum2_ii = (int8_T)(rtConstP.vec_hallToPos_Value[Sum] + 1);
    }

    /* End of Switch: '<S14>/Switch3' */

    /* Switch: '<S14>/Switch2' incorporates:
     *  MinMax: '<S14>/MinMax'
     *  Product: '<S14>/Divide1'
     *  Product: '<S14>/Divide3'
     *  Sum: '<S14>/Sum3'
     */
    if (rtb_LogicalOperator) {
      /* MinMax: '<S14>/MinMax' */
      if (rtDW->Switch3 <= rtDW->z_counterRawPrev) {
        rtDW->Switch2_g = rtDW->Switch3;
      } else {
        rtDW->Switch2_g = rtDW->z_counterRawPrev;
      }

      rtDW->Switch2_g = (int16_T)(((int16_T)((int16_T)((rtDW->Switch2_g << 14) /
        rtDW->z_counterRawPrev) * rtDW->Switch2_a) + (rtb_Sum2_ii << 14)) >> 2);
    } else {
      rtDW->Switch2_g = (int16_T)(rtb_Sum2_ii << 12);
    }

    /* End of Switch: '<S14>/Switch2' */

    /* MinMax: '<S14>/MinMax1' incorporates:
     *  Constant: '<S14>/Constant1'
     *  Switch: '<S14>/Switch2'
     */
    if (rtDW->Switch2_g < 0) {
      rtDW->Switch2_g = 0;
    }

    /* SignalConversion: '<S14>/Signal Conversion2' incorporates:
     *  Merge: '<S3>/Merge'
     *  MinMax: '<S14>/MinMax1'
     *  Product: '<S14>/Divide2'
     */
    rtDW->DataTypeConversion2 = (uint16_T)((15 * rtDW->Switch2_g) >> 3);

    /* End of Outputs for SubSystem: '<S3>/F01_05_Electrical_Angle_Estimation' */
  } else {
    /* Outputs for IfAction SubSystem: '<S3>/F01_06_Electrical_Angle_Measurement' incorporates:
     *  ActionPort: '<S15>/Action Port'
     */
    /* SignalConversion generated from: '<S15>/a_elecAngle' incorporates:
     *  Constant: '<S15>/Constant1'
     *  Constant: '<S15>/Constant3'
     *  Constant: '<S15>/n_polePairs1'
     *  DataTypeConversion: '<S22>/Data Type Conversion1'
     *  DataTypeConversion: '<S22>/Data Type Conversion2'
     *  DataTypeConversion: '<S22>/Data Type Conversion4'
     *  Gain: '<S32>/Gain'
     *  Merge: '<S3>/Merge'
     *  Product: '<S20>/Product'
     *  Product: '<S24>/Product'
     *  Sum: '<S22>/Sum1'
     *  Switch: '<S22>/Switch'
     */
    rtDW->DataTypeConversion2 = (uint16_T)((((uint32_T)(((uint64_T)(uint16_T)
      (((uint32_T)((45ULL * rtP->a_fcpr) >> 6) * rtU->a_encoderCNT) >> 16) << 22)
      / 45ULL) - 23860929U) * rtP->n_polePairs * 45ULL) >> 22);

    /* End of Outputs for SubSystem: '<S3>/F01_06_Electrical_Angle_Measurement' */
  }

  /* End of If: '<S3>/If1' */

  /* If: '<S7>/If1' incorporates:
   *  Constant: '<S1>/z_ctrlTypSel'
   */
  rtb_Sum2_ii = rtDW->If1_ActiveSubsystem;
  UnitDelay3 = -1;
  if (rtP->z_ctrlTypSel == 2) {
    UnitDelay3 = 0;
  }

  rtDW->If1_ActiveSubsystem = UnitDelay3;
  if ((rtb_Sum2_ii != UnitDelay3) && (rtb_Sum2_ii == 0)) {
    /* Disable for If: '<S61>/If2' */
    if (rtDW->If2_ActiveSubsystem_a == 0) {
      /* Disable for Outport: '<S66>/iq' incorporates:
       *  DataTypeConversion: '<S72>/Data Type Conversion'
       * */
      rtDW->DataTypeConversion[0] = 0;

      /* Disable for Abs: '<S66>/Abs5' incorporates:
       *  Outport: '<S66>/iqAbs'
       */
      rtDW->Abs5_d = 0;

      /* Disable for SignalConversion generated from: '<S66>/id' incorporates:
       *  Outport: '<S66>/id'
       */
      rtDW->OutportBufferForid_f = 0;
    }

    rtDW->If2_ActiveSubsystem_a = -1;

    /* End of Disable for If: '<S61>/If2' */

    /* Disable for Interpolation_n-D generated from: '<S68>/r_sin_M1' incorporates:
     *  Outport: '<S61>/r_sin'
     */
    rtDW->r_sin_M1_1 = 0;

    /* Disable for Interpolation_n-D generated from: '<S68>/r_cos_M1' incorporates:
     *  Outport: '<S61>/r_cos'
     */
    rtDW->r_cos_M1_1 = 0;

    /* Disable for Outport: '<Root>/iq' incorporates:
     *  Outport: '<S61>/iq'
     */
    rtY->iq = 0;

    /* Disable for Outport: '<Root>/id' incorporates:
     *  Outport: '<S61>/id'
     */
    rtY->id = 0;

    /* Disable for SignalConversion generated from: '<S61>/iqAbs' incorporates:
     *  Outport: '<S61>/iqAbs'
     */
    rtDW->OutportBufferForiqAbs = 0;
  }

  if (UnitDelay3 == 0) {
    /* Outputs for IfAction SubSystem: '<S7>/Clarke_Park_Transform_Forward' incorporates:
     *  ActionPort: '<S61>/Action Port'
     */
    /* If: '<S65>/If1' incorporates:
     *  Constant: '<S65>/z_selPhaCurMeasABC'
     */
    if (rtP->z_selPhaCurMeasABC == 0) {
      /* Outputs for IfAction SubSystem: '<S65>/Clarke_PhasesAB' incorporates:
       *  ActionPort: '<S69>/Action Port'
       */
      /* Gain: '<S69>/one_by_sqrt3' incorporates:
       *  Merge: '<S65>/Merge1'
       *  Saturate: '<S1>/Saturation'
       *  Saturate: '<S1>/Saturation1'
       *  SignalConversion generated from: '<S69>/i_phaA'
       *  Sum: '<S69>/a_plus_2b'
       */
      rtDW->Switch2_g = (int16_T)(((int16_T)((int16_T)(rtDW->Saturation +
        rtDW->Saturation1) + rtDW->Saturation1) * 18919) >> 15);

      /* End of Outputs for SubSystem: '<S65>/Clarke_PhasesAB' */
    } else if (rtP->z_selPhaCurMeasABC == 1) {
      /* Outputs for IfAction SubSystem: '<S65>/Clarke_PhasesBC' incorporates:
       *  ActionPort: '<S71>/Action Port'
       */
      /* Sum: '<S71>/Sum3' incorporates:
       *  Saturate: '<S1>/Saturation'
       *  Saturate: '<S1>/Saturation1'
       */
      rtDW->sigIdx = rtDW->Saturation - rtDW->Saturation1;
      if (rtDW->sigIdx > 32767) {
        rtDW->sigIdx = 32767;
      } else if (rtDW->sigIdx < -32768) {
        rtDW->sigIdx = -32768;
      }

      /* Gain: '<S71>/Gain2' incorporates:
       *  Merge: '<S65>/Merge1'
       *  Sum: '<S71>/Sum3'
       */
      rtDW->sigIdx *= 18919;
      rtDW->Switch2_g = (int16_T)(((rtDW->sigIdx < 0 ? 32767 : 0) + rtDW->sigIdx)
        >> 15);

      /* Sum: '<S71>/Sum1' incorporates:
       *  Merge: '<S65>/Merge2'
       *  Saturate: '<S1>/Saturation'
       *  Saturate: '<S1>/Saturation1'
       */
      rtDW->sigIdx = -rtDW->Saturation - rtDW->Saturation1;
      if (rtDW->sigIdx > 32767) {
        rtDW->sigIdx = 32767;
      } else if (rtDW->sigIdx < -32768) {
        rtDW->sigIdx = -32768;
      }

      rtDW->Saturation = (int16_T)rtDW->sigIdx;

      /* End of Sum: '<S71>/Sum1' */
      /* End of Outputs for SubSystem: '<S65>/Clarke_PhasesBC' */
    } else {
      /* Outputs for IfAction SubSystem: '<S65>/Clarke_PhasesAC' incorporates:
       *  ActionPort: '<S70>/Action Port'
       */
      /* Gain: '<S70>/one_by_sqrt3' incorporates:
       *  Merge: '<S65>/Merge1'
       *  Saturate: '<S1>/Saturation'
       *  Saturate: '<S1>/Saturation1'
       *  SignalConversion generated from: '<S70>/i_phaA'
       *  Sum: '<S70>/a_plus_2b'
       */
      rtDW->Switch2_g = (int16_T)(((int16_T)((-rtDW->Saturation -
        rtDW->Saturation1) - rtDW->Saturation1) * 18919) >> 15);

      /* End of Outputs for SubSystem: '<S65>/Clarke_PhasesAC' */
    }

    /* End of If: '<S65>/If1' */

    /* PreLookup generated from: '<S68>/a_elecAngle_XA' incorporates:
     *  Merge: '<S3>/Merge'
     */
    rtb_k = plook_u8u16u16n15_even8ca_gs(rtDW->DataTypeConversion2, 0U, &rtb_f);

    /* Interpolation_n-D generated from: '<S68>/r_sin_M1' incorporates:
     *  PreLookup generated from: '<S68>/a_elecAngle_XA'
     */
    rtDW->r_sin_M1_1 = intrp1d_s16s32s32u8u16n15la_s(rtb_k, rtb_f,
      rtConstP.r_sin_M1_1_Table, 180U);

    /* Interpolation_n-D generated from: '<S68>/r_cos_M1' incorporates:
     *  PreLookup generated from: '<S68>/a_elecAngle_XA'
     */
    rtDW->r_cos_M1_1 = intrp1d_s16s32s32u8u16n15la_s(rtb_k, rtb_f,
      rtConstP.r_cos_M1_1_Table, 180U);

    /* If: '<S61>/If2' incorporates:
     *  Constant: '<S66>/cf_currFilt'
     */
    rtb_Sum2_ii = rtDW->If2_ActiveSubsystem_a;
    UnitDelay3 = -1;
    if (rtU->b_motEna) {
      UnitDelay3 = 0;
    }

    rtDW->If2_ActiveSubsystem_a = UnitDelay3;
    if ((rtb_Sum2_ii != UnitDelay3) && (rtb_Sum2_ii == 0)) {
      /* Disable for Outport: '<S66>/iq' incorporates:
       *  DataTypeConversion: '<S72>/Data Type Conversion'
       * */
      rtDW->DataTypeConversion[0] = 0;

      /* Disable for Abs: '<S66>/Abs5' incorporates:
       *  Outport: '<S66>/iqAbs'
       */
      rtDW->Abs5_d = 0;

      /* Disable for SignalConversion generated from: '<S66>/id' incorporates:
       *  Outport: '<S66>/id'
       */
      rtDW->OutportBufferForid_f = 0;
    }

    if (UnitDelay3 == 0) {
      if (rtb_Sum2_ii != 0) {
        /* SystemReset for IfAction SubSystem: '<S61>/Current_Filtering' incorporates:
         *  ActionPort: '<S66>/Action Port'
         */
        /* SystemReset for Atomic SubSystem: '<S66>/Low_Pass_Filter' */
        /* SystemReset for If: '<S61>/If2' */
        Low_Pass_Filter_Reset(&rtDW->Low_Pass_Filter_e);

        /* End of SystemReset for SubSystem: '<S66>/Low_Pass_Filter' */
        /* End of SystemReset for SubSystem: '<S61>/Current_Filtering' */
      }

      /* Sum: '<S67>/Sum6' incorporates:
       *  Interpolation_n-D generated from: '<S68>/r_cos_M1'
       *  Interpolation_n-D generated from: '<S68>/r_sin_M1'
       *  Merge: '<S65>/Merge1'
       *  Merge: '<S65>/Merge2'
       *  Product: '<S67>/Divide1'
       *  Product: '<S67>/Divide4'
       */
      rtDW->sigIdx = (int16_T)((rtDW->Switch2_g * rtDW->r_cos_M1_1) >> 14) -
        (int16_T)((rtDW->Saturation * rtDW->r_sin_M1_1) >> 14);
      if (rtDW->sigIdx > 32767) {
        rtDW->sigIdx = 32767;
      } else if (rtDW->sigIdx < -32768) {
        rtDW->sigIdx = -32768;
      }

      /* Outputs for IfAction SubSystem: '<S61>/Current_Filtering' incorporates:
       *  ActionPort: '<S66>/Action Port'
       */
      /* SignalConversion generated from: '<S66>/Low_Pass_Filter' incorporates:
       *  Sum: '<S67>/Sum6'
       */
      rtDW->TmpSignalConversionAtLow_Pa[0] = (int16_T)rtDW->sigIdx;

      /* End of Outputs for SubSystem: '<S61>/Current_Filtering' */

      /* Sum: '<S67>/Sum1' incorporates:
       *  Interpolation_n-D generated from: '<S68>/r_cos_M1'
       *  Interpolation_n-D generated from: '<S68>/r_sin_M1'
       *  Merge: '<S65>/Merge1'
       *  Merge: '<S65>/Merge2'
       *  Product: '<S67>/Divide2'
       *  Product: '<S67>/Divide3'
       */
      rtDW->sigIdx = (int16_T)((rtDW->Saturation * rtDW->r_cos_M1_1) >> 14) +
        (int16_T)((rtDW->Switch2_g * rtDW->r_sin_M1_1) >> 14);
      if (rtDW->sigIdx > 32767) {
        rtDW->sigIdx = 32767;
      } else if (rtDW->sigIdx < -32768) {
        rtDW->sigIdx = -32768;
      }

      /* Outputs for IfAction SubSystem: '<S61>/Current_Filtering' incorporates:
       *  ActionPort: '<S66>/Action Port'
       */
      /* SignalConversion generated from: '<S66>/Low_Pass_Filter' incorporates:
       *  Sum: '<S67>/Sum1'
       */
      rtDW->TmpSignalConversionAtLow_Pa[1] = (int16_T)rtDW->sigIdx;

      /* Outputs for Atomic SubSystem: '<S66>/Low_Pass_Filter' */
      Low_Pass_Filter(rtDW->TmpSignalConversionAtLow_Pa, rtP->cf_currFilt,
                      rtDW->DataTypeConversion, &rtDW->Low_Pass_Filter_e);

      /* End of Outputs for SubSystem: '<S66>/Low_Pass_Filter' */

      /* Abs: '<S66>/Abs5' incorporates:
       *  Constant: '<S66>/cf_currFilt'
       */
      if (rtDW->DataTypeConversion[0] < 0) {
        /* Abs: '<S66>/Abs5' */
        rtDW->Abs5_d = (int16_T)-rtDW->DataTypeConversion[0];
      } else {
        /* Abs: '<S66>/Abs5' */
        rtDW->Abs5_d = rtDW->DataTypeConversion[0];
      }

      /* End of Abs: '<S66>/Abs5' */

      /* SignalConversion generated from: '<S66>/id' */
      rtDW->OutportBufferForid_f = rtDW->DataTypeConversion[1];

      /* End of Outputs for SubSystem: '<S61>/Current_Filtering' */
    }

    /* End of If: '<S61>/If2' */

    /* Outport: '<Root>/id' incorporates:
     *  SignalConversion generated from: '<S61>/id'
     *  SignalConversion generated from: '<S66>/id'
     */
    rtY->id = rtDW->OutportBufferForid_f;

    /* Outport: '<Root>/iq' incorporates:
     *  SignalConversion generated from: '<S61>/iq'
     */
    rtY->iq = rtDW->DataTypeConversion[0];

    /* SignalConversion generated from: '<S61>/iqAbs' incorporates:
     *  Abs: '<S66>/Abs5'
     */
    rtDW->OutportBufferForiqAbs = rtDW->Abs5_d;

    /* End of Outputs for SubSystem: '<S7>/Clarke_Park_Transform_Forward' */
  }

  /* End of If: '<S7>/If1' */

  /* Chart: '<S1>/Task_Scheduler' incorporates:
   *  Constant: '<S1>/b_cruiseCtrlEna'
   *  Constant: '<S1>/z_ctrlTypSel'
   *  UnitDelay: '<S2>/UnitDelay2'
   *  UnitDelay: '<S2>/UnitDelay5'
   *  UnitDelay: '<S2>/UnitDelay6'
   *  UnitDelay: '<S8>/UnitDelay4'
   */
  if (rtDW->UnitDelay2_DSTATE_g) {
    /* Outputs for Function Call SubSystem: '<S1>/F02_Diagnostics' */
    /* If: '<S4>/If2' incorporates:
     *  Constant: '<S33>/CTRL_COMM2'
     *  Constant: '<S33>/t_errDequal'
     *  Constant: '<S33>/t_errQual'
     *  Constant: '<S4>/b_diagEna'
     *  RelationalOperator: '<S33>/Relational Operator2'
     */
    if (rtP->b_diagEna) {
      /* Outputs for IfAction SubSystem: '<S4>/Diagnostics_Enabled' incorporates:
       *  ActionPort: '<S33>/Action Port'
       */
      /* Switch: '<S33>/Switch3' incorporates:
       *  Abs: '<S13>/Abs5'
       *  Abs: '<S33>/Abs4'
       *  Constant: '<S13>/n_stdStillDet'
       *  Constant: '<S33>/CTRL_COMM4'
       *  Constant: '<S33>/r_errInpTgtThres'
       *  Logic: '<S33>/Logical Operator1'
       *  RelationalOperator: '<S13>/Relational Operator9'
       *  RelationalOperator: '<S33>/Relational Operator7'
       *  S-Function (sfix_bitop): '<S33>/Bitwise Operator1'
       *  UnitDelay: '<S33>/UnitDelay'
       *  UnitDelay: '<S8>/UnitDelay4'
       */
      if ((rtDW->Switch1 & 4U) != 0U) {
        rtb_RelationalOperator = true;
      } else {
        if (rtDW->UnitDelay4_DSTATE_a < 0) {
          /* Abs: '<S33>/Abs4' incorporates:
           *  UnitDelay: '<S8>/UnitDelay4'
           */
          rtDW->Switch2_g = (int16_T)-rtDW->UnitDelay4_DSTATE_a;
        } else {
          /* Abs: '<S33>/Abs4' incorporates:
           *  UnitDelay: '<S8>/UnitDelay4'
           */
          rtDW->Switch2_g = rtDW->UnitDelay4_DSTATE_a;
        }

        rtb_RelationalOperator = (rtU->b_motEna && (Abs5 < rtP->n_stdStillDet) &&
          (rtDW->Switch2_g > rtP->r_errInpTgtThres));
      }

      /* End of Switch: '<S33>/Switch3' */

      /* Sum: '<S33>/Sum' incorporates:
       *  Constant: '<S33>/CTRL_COMM'
       *  Constant: '<S33>/CTRL_COMM1'
       *  DataTypeConversion: '<S33>/Data Type Conversion3'
       *  Gain: '<S33>/g_Hb'
       *  Gain: '<S33>/g_Hb1'
       *  RelationalOperator: '<S33>/Relational Operator1'
       *  RelationalOperator: '<S33>/Relational Operator3'
       */
      rtb_k = (uint8_T)(((uint32_T)((Sum == 7) << 1) + (uint32_T)(Sum == 0)) +
                        (uint32_T)(rtb_RelationalOperator << 2));

      /* Outputs for Atomic SubSystem: '<S33>/Debounce_Filter' */
      Debounce_Filter((rtb_k != 0), rtP->t_errQual, rtP->t_errDequal,
                      &rtDW->Merge_a, &rtDW->Debounce_Filter_e);

      /* End of Outputs for SubSystem: '<S33>/Debounce_Filter' */

      /* Outputs for Atomic SubSystem: '<S33>/either_edge' */
      rtb_RelationalOperator = either_edge(rtDW->Merge_a, &rtDW->either_edge_j);

      /* End of Outputs for SubSystem: '<S33>/either_edge' */

      /* Switch: '<S33>/Switch1' incorporates:
       *  Constant: '<S33>/CTRL_COMM2'
       *  Constant: '<S33>/t_errDequal'
       *  Constant: '<S33>/t_errQual'
       *  RelationalOperator: '<S33>/Relational Operator2'
       */
      if (rtb_RelationalOperator) {
        /* Switch: '<S33>/Switch1' */
        rtDW->Switch1 = rtb_k;
      }

      /* End of Switch: '<S33>/Switch1' */
      /* End of Outputs for SubSystem: '<S4>/Diagnostics_Enabled' */
    }

    /* End of If: '<S4>/If2' */

    /* SignalConversion generated from: '<S4>/b_errFlag' */
    rtDW->OutportBufferForb_errFlag = rtDW->Merge_a;

    /* Outport: '<Root>/z_errCode' incorporates:
     *  SignalConversion generated from: '<S4>/z_errCode '
     */
    rtY->z_errCode = rtDW->Switch1;

    /* End of Outputs for SubSystem: '<S1>/F02_Diagnostics' */

    /* Outputs for Function Call SubSystem: '<S1>/F03_Control_Mode_Manager' */
    F03_Control_Mode_Manager(rtDW->OutportBufferForb_errFlag, rtU->b_motEna,
      rtP->b_cruiseCtrlEna, rtP->z_ctrlTypSel, rtU->z_ctrlModReq, rtU->r_inpTgt,
      rtDW->UnitDelay4_DSTATE_a, &rtDW->z_ctrlMod, &rtDW->Abs1, &rtDW->Merge1,
      &rtDW->Saturation, rtP, &rtDW->F03_Control_Mode_Manager_f);

    /* End of Outputs for SubSystem: '<S1>/F03_Control_Mode_Manager' */

    /* Outputs for Function Call SubSystem: '<S7>/FOC' */
    FOC(rtP->z_ctrlTypSel, rtb_LogicalOperator, rtDW->z_ctrlMod, rtDW->Merge1,
        rtDW->Abs1, Switch2, rtDW->OutportBufferForr_fieldWeak, rtY->id, rtY->iq,
        rtDW->VariantMergeForOutportVd_max, rtDW->VariantMergeForOutportVd_min,
        rtDW->VariantMergeForOutportVq_max, rtDW->VariantMergeForOutportVq_min,
        rtDW->OutportBufferForid_max, rtDW->Gain4, rtDW->Divide1_d, rtDW->Gain1,
        rtDW->Switch2_b, rtDW->Switch2_p, rtDW->Divide1, rtDW->Switch2_m,
        rtDW->UnitDelay4_DSTATE_a, rtP->b_cruiseCtrlEna, rtP, &rtDW->Merge,
        &rtDW->VariantMergeForOutportVq_Trq, &rtDW->FOC_h);

    /* End of Outputs for SubSystem: '<S7>/FOC' */
  } else if (rtDW->UnitDelay5_DSTATE_l) {
    /* Outputs for Function Call SubSystem: '<S1>/F03_Control_Mode_Manager' */
    F03_Control_Mode_Manager(rtDW->OutportBufferForb_errFlag, rtU->b_motEna,
      rtP->b_cruiseCtrlEna, rtP->z_ctrlTypSel, rtU->z_ctrlModReq, rtU->r_inpTgt,
      rtDW->UnitDelay4_DSTATE_a, &rtDW->z_ctrlMod, &rtDW->Abs1, &rtDW->Merge1,
      &rtDW->Saturation, rtP, &rtDW->F03_Control_Mode_Manager_f);

    /* End of Outputs for SubSystem: '<S1>/F03_Control_Mode_Manager' */

    /* Outputs for Function Call SubSystem: '<S1>/F04_Field_Weakening' */
    /* If: '<S6>/If3' incorporates:
     *  Constant: '<S1>/b_cruiseCtrlEna'
     *  Constant: '<S1>/z_ctrlTypSel'
     *  Constant: '<S6>/b_fieldWeakEna'
     *  UnitDelay: '<S8>/UnitDelay4'
     */
    if (rtP->b_fieldWeakEna) {
      /* Outputs for IfAction SubSystem: '<S6>/Field_Weakening_Enabled' incorporates:
       *  ActionPort: '<S58>/Action Port'
       */
      /* Abs: '<S58>/Abs5' incorporates:
       *  DataTypeConversion: '<S46>/Data Type Conversion22'
       */
      if (rtDW->Saturation < 0) {
        rtDW->Saturation = (int16_T)-rtDW->Saturation;
      }

      /* End of Abs: '<S58>/Abs5' */

      /* Switch: '<S60>/Switch2' incorporates:
       *  Abs: '<S58>/Abs5'
       *  Constant: '<S58>/r_fieldWeakHi'
       *  Constant: '<S58>/r_fieldWeakLo'
       *  RelationalOperator: '<S60>/LowerRelop1'
       *  RelationalOperator: '<S60>/UpperRelop'
       *  Sum: '<S58>/Sum4'
       *  Switch: '<S60>/Switch'
       */
      if (rtDW->Saturation > rtP->r_fieldWeakHi) {
        rtDW->Saturation = rtP->r_fieldWeakHi;
      } else if (rtDW->Saturation < rtP->r_fieldWeakLo) {
        /* Switch: '<S60>/Switch' incorporates:
         *  Constant: '<S58>/r_fieldWeakLo'
         *  Sum: '<S58>/Sum4'
         */
        rtDW->Saturation = rtP->r_fieldWeakLo;
      }

      /* End of Switch: '<S60>/Switch2' */

      /* Product: '<S58>/Divide14' incorporates:
       *  Constant: '<S58>/r_fieldWeakHi'
       *  Constant: '<S58>/r_fieldWeakLo'
       *  Sum: '<S58>/Sum1'
       *  Sum: '<S58>/Sum3'
       *  Sum: '<S58>/Sum4'
       */
      rtb_f = (uint16_T)(((int16_T)(rtDW->Saturation - rtP->r_fieldWeakLo) << 15)
                         / (int16_T)(rtP->r_fieldWeakHi - rtP->r_fieldWeakLo));

      /* Switch: '<S59>/Switch2' incorporates:
       *  Abs: '<S13>/Abs5'
       *  Constant: '<S58>/n_fieldWeakAuthHi'
       *  Constant: '<S58>/n_fieldWeakAuthLo'
       *  RelationalOperator: '<S59>/LowerRelop1'
       *  RelationalOperator: '<S59>/UpperRelop'
       *  Switch: '<S59>/Switch'
       */
      if (Abs5 > rtP->n_fieldWeakAuthHi) {
        rtDW->Switch2_g = rtP->n_fieldWeakAuthHi;
      } else if (Abs5 < rtP->n_fieldWeakAuthLo) {
        /* Switch: '<S59>/Switch' incorporates:
         *  Constant: '<S58>/n_fieldWeakAuthLo'
         */
        rtDW->Switch2_g = rtP->n_fieldWeakAuthLo;
      } else {
        rtDW->Switch2_g = Abs5;
      }

      /* Product: '<S58>/Divide1' incorporates:
       *  Constant: '<S58>/n_fieldWeakAuthHi'
       *  Constant: '<S58>/n_fieldWeakAuthLo'
       *  Sum: '<S58>/Sum2'
       *  Sum: '<S58>/Sum4'
       *  Switch: '<S59>/Switch2'
       */
      rtb_Divide1 = (uint16_T)(((int16_T)(rtDW->Switch2_g -
        rtP->n_fieldWeakAuthLo) << 15) / (int16_T)(rtP->n_fieldWeakAuthHi -
        rtP->n_fieldWeakAuthLo));

      /* Switch: '<S58>/Switch1' incorporates:
       *  MinMax: '<S58>/MinMax1'
       *  Product: '<S58>/Divide1'
       *  Product: '<S58>/Divide14'
       *  RelationalOperator: '<S58>/Relational Operator6'
       */
      if (rtb_f < rtb_Divide1) {
        /* MinMax: '<S58>/MinMax' incorporates:
         *  Switch: '<S58>/Switch1'
         */
        if (rtb_f < rtb_Divide1) {
          rtb_f = rtb_Divide1;
        }

        /* End of MinMax: '<S58>/MinMax' */
      } else if (rtb_Divide1 <= rtb_f) {
        /* MinMax: '<S58>/MinMax1' incorporates:
         *  Switch: '<S58>/Switch1'
         */
        rtb_f = rtb_Divide1;
      }

      /* End of Switch: '<S58>/Switch1' */

      /* Switch: '<S58>/Switch2' incorporates:
       *  Constant: '<S58>/CTRL_COMM2'
       *  Constant: '<S58>/a_phaAdvMax'
       *  Constant: '<S58>/id_fieldWeakMax'
       *  RelationalOperator: '<S58>/Relational Operator1'
       */
      if (rtP->z_ctrlTypSel == 2) {
        rtDW->Switch2_g = rtP->id_fieldWeakMax;
      } else {
        rtDW->Switch2_g = rtP->a_phaAdvMax;
      }

      /* Product: '<S58>/Divide3' incorporates:
       *  Switch: '<S58>/Switch1'
       *  Switch: '<S58>/Switch2'
       */
      rtDW->Divide3 = (int16_T)((rtDW->Switch2_g * rtb_f) >> 15);

      /* End of Outputs for SubSystem: '<S6>/Field_Weakening_Enabled' */
    }

    /* End of If: '<S6>/If3' */

    /* SignalConversion generated from: '<S6>/r_fieldWeak ' incorporates:
     *  Product: '<S58>/Divide3'
     */
    rtDW->OutportBufferForr_fieldWeak = rtDW->Divide3;

    /* End of Outputs for SubSystem: '<S1>/F04_Field_Weakening' */

    /* Outputs for Function Call SubSystem: '<S7>/Motor_Limitations' */
    /* If: '<S64>/If1' incorporates:
     *  Constant: '<S1>/z_ctrlTypSel'
     */
    rtb_Sum2_ii = rtDW->If1_ActiveSubsystem_b;
    UnitDelay3 = -1;
    if (rtP->z_ctrlTypSel == 2) {
      UnitDelay3 = 0;
    }

    rtDW->If1_ActiveSubsystem_b = UnitDelay3;
    if ((rtb_Sum2_ii != UnitDelay3) && (rtb_Sum2_ii == 0)) {
      /* Disable for SwitchCase: '<S216>/Switch Case' */
      rtDW->SwitchCase_ActiveSubsystem = -1;
    }

    if (UnitDelay3 == 0) {
      /* Outputs for IfAction SubSystem: '<S64>/Motor_Limitations_Enabled' incorporates:
       *  ActionPort: '<S216>/Action Port'
       */
      /* Product: '<S216>/Divide4' incorporates:
       *  Constant: '<S216>/i_max'
       *  SignalConversion generated from: '<S6>/r_fieldWeak '
       */
      rtDW->sigIdx = rtDW->OutportBufferForr_fieldWeak << 16;
      rtDW->sigIdx = (rtDW->sigIdx == MIN_int32_T) && (rtP->i_max == -1) ?
        MAX_int32_T : rtDW->sigIdx / rtP->i_max;
      if (rtDW->sigIdx < 0) {
        rtDW->sigIdx = 0;
      } else if (rtDW->sigIdx > 65535) {
        rtDW->sigIdx = 65535;
      }

      /* Product: '<S216>/Divide1' incorporates:
       *  Constant: '<S216>/i_max'
       *  Interpolation_n-D: '<S216>/iq_maxSca_M1'
       *  PreLookup: '<S216>/iq_maxSca_XA'
       *  Product: '<S216>/Divide4'
       */
      rtDW->Divide1_d = (int16_T)
        ((rtConstP.iq_maxSca_M1_Table[plook_u8u16_evenckag((uint16_T)
           rtDW->sigIdx, 0U, 655U)] * rtP->i_max) >> 16);

      /* Gain: '<S216>/Gain1' incorporates:
       *  Product: '<S216>/Divide1'
       */
      rtDW->Gain1 = (int16_T)-rtDW->Divide1_d;

      /* Outputs for Atomic SubSystem: '<S216>/Variant Subsystem' */
#if mcu_model == 1

      /* Outputs for Atomic SubSystem: '<S219>/GD32F103' */
      /* Abs: '<S279>/Abs5' incorporates:
       *  UnitDelay: '<S7>/UnitDelay4'
       */
      if (rtDW->VariantMergeForOutportVq_Trq < 0) {
        rtDW->Switch2_g = (int16_T)-rtDW->VariantMergeForOutportVq_Trq;
      } else {
        rtDW->Switch2_g = rtDW->VariantMergeForOutportVq_Trq;
      }

      /* VariantMerge generated from: '<S219>/Vq_max' incorporates:
       *  Abs: '<S279>/Abs5'
       *  Interpolation_n-D: '<S279>/GD32_Vq_max_M1'
       *  PreLookup: '<S279>/GD32_Vq_max_XA'
       */
      rtDW->VariantMergeForOutportVq_max =
        rtConstP.GD32_Vq_max_M1_Table[plook_u8s16_evenckag(rtDW->Switch2_g, 0,
        320U)];

      /* VariantMerge generated from: '<S219>/Vd_min' incorporates:
       *  Gain: '<S279>/Gain5'
       *  VariantMerge generated from: '<S219>/Vq_max'
       */
      rtDW->VariantMergeForOutportVd_min = (int16_T)
        -rtDW->VariantMergeForOutportVq_max;

      /* VariantMerge generated from: '<S219>/Vd_max' incorporates:
       *  Constant: '<S279>/Vd_max1'
       */
      rtDW->VariantMergeForOutportVd_max = 26032;

      /* VariantMerge generated from: '<S219>/Vq_min' incorporates:
       *  Gain: '<S279>/Gain3'
       */
      rtDW->VariantMergeForOutportVq_min = -26032;

      /* End of Outputs for SubSystem: '<S219>/GD32F103' */
#elif mcu_model == 0

      /* Outputs for Atomic SubSystem: '<S219>/STM32F103' */
      /* Abs: '<S280>/Abs5' incorporates:
       *  UnitDelay: '<S7>/UnitDelay4'
       */
      if (rtDW->VariantMergeForOutportVq_Trq < 0) {
        rtDW->Switch2_g = (int16_T)-rtDW->VariantMergeForOutportVq_Trq;
      } else {
        rtDW->Switch2_g = rtDW->VariantMergeForOutportVq_Trq;
      }

      /* VariantMerge generated from: '<S219>/Vq_max' incorporates:
       *  Abs: '<S280>/Abs5'
       *  Interpolation_n-D: '<S280>/STM32_Vq_max_M1'
       *  PreLookup: '<S280>/STM32_Vq_max_XA'
       */
      rtDW->VariantMergeForOutportVq_max =
        rtConstP.STM32_Vq_max_M1_Table[plook_u8s16_evenckag(rtDW->Switch2_g, 0,
        480U)];

      /* VariantMerge generated from: '<S219>/Vd_min' incorporates:
       *  Gain: '<S280>/Gain5'
       *  VariantMerge generated from: '<S219>/Vq_max'
       */
      rtDW->VariantMergeForOutportVd_min = (int16_T)
        -rtDW->VariantMergeForOutportVq_max;

      /* VariantMerge generated from: '<S219>/Vd_max' incorporates:
       *  Constant: '<S280>/Vd_max2'
       */
      rtDW->VariantMergeForOutportVd_max = 15040;

      /* VariantMerge generated from: '<S219>/Vq_min' incorporates:
       *  Gain: '<S280>/Gain3'
       */
      rtDW->VariantMergeForOutportVq_min = -15040;

      /* End of Outputs for SubSystem: '<S219>/STM32F103' */
#endif

      /* End of Outputs for SubSystem: '<S216>/Variant Subsystem' */

      /* SwitchCase: '<S216>/Switch Case' incorporates:
       *  Abs: '<S13>/Abs5'
       *  Constant: '<S216>/n_max'
       *  Constant: '<S220>/Constant'
       *  Constant: '<S220>/Constant1'
       *  Constant: '<S220>/cf_KbLimProt'
       *  Constant: '<S220>/cf_iqKiLimProt'
       *  Constant: '<S220>/cf_nKiLimProt'
       *  Constant: '<S277>/Vd_max1'
       *  Constant: '<S278>/Vd_max3'
       *  Product: '<S216>/Divide1'
       *  SignalConversion generated from: '<S61>/iqAbs'
       *  Sum: '<S220>/Sum1'
       *  Sum: '<S220>/Sum2'
       *  VariantMerge generated from: '<S223>/Vd_max_margin'
       */
      rtb_Sum2_ii = rtDW->SwitchCase_ActiveSubsystem;
      UnitDelay3 = -1;
      switch (rtDW->z_ctrlMod) {
       case 1:
        UnitDelay3 = 0;
        break;

       case 2:
        UnitDelay3 = 1;
        break;

       case 3:
        UnitDelay3 = 2;
        break;
      }

      rtDW->SwitchCase_ActiveSubsystem = UnitDelay3;
      switch (UnitDelay3) {
       case 0:
        if (UnitDelay3 != rtb_Sum2_ii) {
          /* SystemReset for IfAction SubSystem: '<S216>/Voltage_Mode_Protection' incorporates:
           *  ActionPort: '<S220>/Action Port'
           */
          /* SystemReset for Atomic SubSystem: '<S220>/I_backCalc_fixdt' */
          /* SystemReset for SwitchCase: '<S216>/Switch Case' */
          I_backCalc_fixdt_Reset(65536000, &rtDW->I_backCalc_fixdt_c);

          /* End of SystemReset for SubSystem: '<S220>/I_backCalc_fixdt' */

          /* SystemReset for Atomic SubSystem: '<S220>/I_backCalc_fixdt1' */
          I_backCalc_fixdt_Reset(65536000, &rtDW->I_backCalc_fixdt1);

          /* End of SystemReset for SubSystem: '<S220>/I_backCalc_fixdt1' */
          /* End of SystemReset for SubSystem: '<S216>/Voltage_Mode_Protection' */
        }

        /* Outputs for IfAction SubSystem: '<S216>/Voltage_Mode_Protection' incorporates:
         *  ActionPort: '<S220>/Action Port'
         */
        /* Outputs for Atomic SubSystem: '<S220>/I_backCalc_fixdt' */
        I_backCalc_fixdt((int16_T)(rtDW->Divide1_d - rtDW->OutportBufferForiqAbs),
                         rtP->cf_iqKiLimProt, cf_KbLimProt, rtDW->Abs1, 0,
                         &rtDW->Switch2_b, &rtDW->I_backCalc_fixdt_c);

        /* End of Outputs for SubSystem: '<S220>/I_backCalc_fixdt' */

        /* Outputs for Atomic SubSystem: '<S220>/I_backCalc_fixdt1' */
        I_backCalc_fixdt((int16_T)(rtP->n_max - Abs5), rtP->cf_nKiLimProt,
                         cf_KbLimProt, rtDW->Abs1, 0, &rtDW->Switch2_p,
                         &rtDW->I_backCalc_fixdt1);

        /* End of Outputs for SubSystem: '<S220>/I_backCalc_fixdt1' */
        /* End of Outputs for SubSystem: '<S216>/Voltage_Mode_Protection' */
        break;

       case 1:
        /* Outputs for IfAction SubSystem: '<S216>/Speed_Mode_Protection' incorporates:
         *  ActionPort: '<S217>/Action Port'
         */
        /* Switch: '<S221>/Switch2' incorporates:
         *  Gain: '<S216>/Gain1'
         *  Product: '<S216>/Divide1'
         *  RelationalOperator: '<S221>/LowerRelop1'
         *  RelationalOperator: '<S221>/UpperRelop'
         *  Switch: '<S221>/Switch'
         */
        if (rtY->iq > rtDW->Divide1_d) {
          rtDW->Switch2_g = rtDW->Divide1_d;
        } else if (rtY->iq < rtDW->Gain1) {
          /* Switch: '<S221>/Switch' incorporates:
           *  Gain: '<S216>/Gain1'
           */
          rtDW->Switch2_g = rtDW->Gain1;
        } else {
          rtDW->Switch2_g = rtY->iq;
        }

        /* Product: '<S217>/Divide1' incorporates:
         *  Constant: '<S217>/cf_iqKiLimProt'
         *  Sum: '<S217>/Sum3'
         *  Switch: '<S221>/Switch2'
         */
        rtDW->Divide1 = (int16_T)(rtDW->Switch2_g - rtY->iq) *
          rtP->cf_iqKiLimProt;

        /* End of Outputs for SubSystem: '<S216>/Speed_Mode_Protection' */
        break;

       case 2:
        if (UnitDelay3 != rtb_Sum2_ii) {
          /* InitializeConditions for IfAction SubSystem: '<S216>/Torque_Mode_Protection' incorporates:
           *  ActionPort: '<S218>/Action Port'
           */
          /* InitializeConditions for SwitchCase: '<S216>/Switch Case' incorporates:
           *  DiscreteIntegrator: '<S258>/Integrator'
           */
          rtDW->Integrator_IC_LOADING = 1U;

          /* End of InitializeConditions for SubSystem: '<S216>/Torque_Mode_Protection' */
        }

        /* Outputs for IfAction SubSystem: '<S216>/Torque_Mode_Protection' incorporates:
         *  ActionPort: '<S218>/Action Port'
         */
        /* Outputs for Atomic SubSystem: '<S218>/Variant Subsystem1' */
#if mcu_model == 1

        /* Outputs for Atomic SubSystem: '<S223>/GD32F103' */
        rtb_Switch2_j = 26032;

        /* End of Outputs for SubSystem: '<S223>/GD32F103' */
#endif

#if mcu_model == 0

        /* Outputs for Atomic SubSystem: '<S223>/STM32F103' */
        rtb_Switch2_j = 15040;

        /* End of Outputs for SubSystem: '<S223>/STM32F103' */
#endif

        /* End of Outputs for SubSystem: '<S218>/Variant Subsystem1' */

#ifdef ENCODER_SPEED_LIMIT_ENABLE
        /* The MT6701 build applies its speed ceiling to r_inpTgt before this
         * generated controller is stepped.  The legacy torque-mode limiter
         * below uses Hall-derived speed (zero with emulated static Halls) and
         * has a low-RPM fixed-point ratchet.  Keep the voltage ceiling fully
         * open here while retaining all FOC current and voltage saturation. */
        rtDW->Switch2_m = rtb_Switch2_j;
        rtDW->Integrator_DSTATE = rtb_Switch2_j;
        rtDW->Integrator_IC_LOADING = 0U;
#else

        /* DiscreteIntegrator: '<S258>/Integrator' incorporates:
         *  Constant: '<S277>/Vd_max1'
         *  Constant: '<S278>/Vd_max3'
         *  VariantMerge generated from: '<S223>/Vd_max_margin'
         */
        if (rtDW->Integrator_IC_LOADING != 0) {
          rtDW->Integrator_DSTATE = rtb_Switch2_j;
        }

        /* Switch: '<S266>/Switch2' incorporates:
         *  DiscreteIntegrator: '<S258>/Integrator'
         *  RelationalOperator: '<S266>/LowerRelop1'
         *  RelationalOperator: '<S266>/UpperRelop'
         *  Switch: '<S266>/Switch'
         *  VariantMerge generated from: '<S219>/Vq_max'
         */
        if (rtDW->Integrator_DSTATE > rtDW->VariantMergeForOutportVq_max) {
          /* Switch: '<S266>/Switch2' */
          rtDW->Switch2_m = rtDW->VariantMergeForOutportVq_max;
        } else if (rtDW->Integrator_DSTATE < 0) {
          /* Switch: '<S266>/Switch' incorporates:
           *  Constant: '<S218>/Constant1'
           *  Switch: '<S266>/Switch2'
           */
          rtDW->Switch2_m = 0;
        } else {
          /* Switch: '<S266>/Switch2' */
          rtDW->Switch2_m = rtDW->Integrator_DSTATE;
        }

        /* End of Switch: '<S266>/Switch2' */

        /* Update for DiscreteIntegrator: '<S258>/Integrator' */
        rtDW->Integrator_IC_LOADING = 0U;

        /* Sum: '<S250>/SumI2' incorporates:
         *  DiscreteIntegrator: '<S258>/Integrator'
         *  Switch: '<S266>/Switch2'
         */
        rtDW->sigIdx = rtDW->Switch2_m - rtDW->Integrator_DSTATE;
        if (rtDW->sigIdx > 32767) {
          rtDW->sigIdx = 32767;
        } else if (rtDW->sigIdx < -32768) {
          rtDW->sigIdx = -32768;
        }

        /* Gain: '<S250>/Kb' */
        if (cf_KbLimProt > 255) {
          rtDW->Switch2_g = MAX_int16_T;
        } else {
          rtDW->Switch2_g = (int16_T)(cf_KbLimProt << 7);
        }

        /* Sum: '<S250>/SumI4' incorporates:
         *  Abs: '<S13>/Abs5'
         *  Constant: '<S216>/n_max'
         *  Constant: '<S218>/cf_nKiLimProt'
         *  Gain: '<S250>/Kb'
         *  Product: '<S255>/IProd Out'
         *  Sum: '<S218>/Sum1'
         *  Sum: '<S250>/SumI2'
         */
        rtDW->i = (((int64_T)(((int16_T)(rtP->n_max - Abs5) * rtP->cf_nKiLimProt) >>
                              16) << 19) + rtDW->Switch2_g * rtDW->sigIdx) >> 3;
        if (rtDW->i > 2147483647LL) {
          rtDW->i = 2147483647LL;
        } else if (rtDW->i < -2147483648LL) {
          rtDW->i = -2147483648LL;
        }

        /* Update for DiscreteIntegrator: '<S258>/Integrator' incorporates:
         *  Sum: '<S250>/SumI4'
         */
        rtDW->sigIdx = ((int32_T)rtDW->i >> 16) + rtDW->Integrator_DSTATE;
        if (rtDW->sigIdx > 32767) {
          rtDW->sigIdx = 32767;
        } else if (rtDW->sigIdx < -32768) {
          rtDW->sigIdx = -32768;
        }

        rtDW->Integrator_DSTATE = (int16_T)rtDW->sigIdx;
#endif

        /* End of Outputs for SubSystem: '<S216>/Torque_Mode_Protection' */
        break;
      }

      /* End of SwitchCase: '<S216>/Switch Case' */

      /* Gain: '<S216>/Gain4' incorporates:
       *  Constant: '<S216>/i_max'
       */
      rtDW->Gain4 = (int16_T)-rtP->i_max;

      /* SignalConversion generated from: '<S216>/id_max' incorporates:
       *  Constant: '<S216>/i_max'
       */
      rtDW->OutportBufferForid_max = rtP->i_max;

      /* End of Outputs for SubSystem: '<S64>/Motor_Limitations_Enabled' */
    }

    /* End of If: '<S64>/If1' */
    /* End of Outputs for SubSystem: '<S7>/Motor_Limitations' */

    /* Outputs for Function Call SubSystem: '<S7>/FOC' */
    FOC(rtP->z_ctrlTypSel, rtb_LogicalOperator, rtDW->z_ctrlMod, rtDW->Merge1,
        rtDW->Abs1, Switch2, rtDW->OutportBufferForr_fieldWeak, rtY->id, rtY->iq,
        rtDW->VariantMergeForOutportVd_max, rtDW->VariantMergeForOutportVd_min,
        rtDW->VariantMergeForOutportVq_max, rtDW->VariantMergeForOutportVq_min,
        rtDW->OutportBufferForid_max, rtDW->Gain4, rtDW->Divide1_d, rtDW->Gain1,
        rtDW->Switch2_b, rtDW->Switch2_p, rtDW->Divide1, rtDW->Switch2_m,
        rtDW->UnitDelay4_DSTATE_a, rtP->b_cruiseCtrlEna, rtP, &rtDW->Merge,
        &rtDW->VariantMergeForOutportVq_Trq, &rtDW->FOC_h);

    /* End of Outputs for SubSystem: '<S7>/FOC' */
  } else if (rtDW->UnitDelay6_DSTATE) {
    /* Outputs for Function Call SubSystem: '<S1>/F03_Control_Mode_Manager' */
    F03_Control_Mode_Manager(rtDW->OutportBufferForb_errFlag, rtU->b_motEna,
      rtP->b_cruiseCtrlEna, rtP->z_ctrlTypSel, rtU->z_ctrlModReq, rtU->r_inpTgt,
      rtDW->UnitDelay4_DSTATE_a, &rtDW->z_ctrlMod, &rtDW->Abs1, &rtDW->Merge1,
      &rtDW->Saturation, rtP, &rtDW->F03_Control_Mode_Manager_f);

    /* End of Outputs for SubSystem: '<S1>/F03_Control_Mode_Manager' */

    /* Outputs for Function Call SubSystem: '<S7>/FOC' */
    FOC(rtP->z_ctrlTypSel, rtb_LogicalOperator, rtDW->z_ctrlMod, rtDW->Merge1,
        rtDW->Abs1, Switch2, rtDW->OutportBufferForr_fieldWeak, rtY->id, rtY->iq,
        rtDW->VariantMergeForOutportVd_max, rtDW->VariantMergeForOutportVd_min,
        rtDW->VariantMergeForOutportVq_max, rtDW->VariantMergeForOutportVq_min,
        rtDW->OutportBufferForid_max, rtDW->Gain4, rtDW->Divide1_d, rtDW->Gain1,
        rtDW->Switch2_b, rtDW->Switch2_p, rtDW->Divide1, rtDW->Switch2_m,
        rtDW->UnitDelay4_DSTATE_a, rtP->b_cruiseCtrlEna, rtP, &rtDW->Merge,
        &rtDW->VariantMergeForOutportVq_Trq, &rtDW->FOC_h);

    /* End of Outputs for SubSystem: '<S7>/FOC' */
  }

  /* End of Chart: '<S1>/Task_Scheduler' */

  /* If: '<S7>/If2' incorporates:
   *  Constant: '<S1>/z_ctrlTypSel'
   *  Constant: '<S8>/CTRL_COMM1'
   *  Merge: '<S46>/Merge1'
   *  Merge: '<S75>/Merge'
   *  RelationalOperator: '<S8>/Relational Operator6'
   *  Switch: '<S8>/Switch2'
   */
  rtb_Sum2_ii = rtDW->If2_ActiveSubsystem;
  UnitDelay3 = -1;
  if (rtP->z_ctrlTypSel == 2) {
    rtb_Switch2_j = rtDW->Merge;
    UnitDelay3 = 0;
  } else {
    rtb_Switch2_j = rtDW->Merge1;
  }

  rtDW->If2_ActiveSubsystem = UnitDelay3;
  if ((rtb_Sum2_ii != UnitDelay3) && (rtb_Sum2_ii == 0)) {
    /* Disable for Gain: '<S73>/Gain4' incorporates:
     *  Outport: '<S62>/V_phaABC_FOC'
     */
    rtDW->Gain4_o[0] = 0;
    rtDW->Gain4_o[1] = 0;
    rtDW->Gain4_o[2] = 0;
  }

  if (UnitDelay3 == 0) {
    /* Outputs for IfAction SubSystem: '<S7>/Clarke_Park_Transform_Inverse' incorporates:
     *  ActionPort: '<S62>/Action Port'
     */
    /* Sum: '<S74>/Sum6' incorporates:
     *  Interpolation_n-D generated from: '<S68>/r_cos_M1'
     *  Interpolation_n-D generated from: '<S68>/r_sin_M1'
     *  Merge: '<S75>/Merge'
     *  Product: '<S74>/Divide1'
     *  Product: '<S74>/Divide4'
     *  VariantMerge generated from: '<S150>/Vq_Trq'
     */
    rtDW->sigIdx = (int16_T)((rtDW->VariantMergeForOutportVq_Trq *
      rtDW->r_cos_M1_1) >> 14) - (int16_T)((rtDW->Merge * rtDW->r_sin_M1_1) >>
      14);
    if (rtDW->sigIdx > 32767) {
      rtDW->sigIdx = 32767;
    } else if (rtDW->sigIdx < -32768) {
      rtDW->sigIdx = -32768;
    }

    /* Sum: '<S74>/Sum1' incorporates:
     *  Interpolation_n-D generated from: '<S68>/r_cos_M1'
     *  Interpolation_n-D generated from: '<S68>/r_sin_M1'
     *  Merge: '<S75>/Merge'
     *  Product: '<S74>/Divide2'
     *  Product: '<S74>/Divide3'
     *  VariantMerge generated from: '<S150>/Vq_Trq'
     */
    rtDW->i3 = (int16_T)((rtDW->VariantMergeForOutportVq_Trq * rtDW->r_sin_M1_1)
                         >> 14) + (int16_T)((rtDW->Merge * rtDW->r_cos_M1_1) >>
      14);
    if (rtDW->i3 > 32767) {
      rtDW->i3 = 32767;
    } else if (rtDW->i3 < -32768) {
      rtDW->i3 = -32768;
    }

    /* Gain: '<S73>/Gain1' incorporates:
     *  Sum: '<S74>/Sum1'
     */
    rtDW->i3 *= 14189;

    /* Sum: '<S73>/Sum6' incorporates:
     *  Gain: '<S73>/Gain1'
     *  Gain: '<S73>/Gain3'
     *  Sum: '<S74>/Sum6'
     */
    rtDW->i3 = (((rtDW->i3 < 0 ? 16383 : 0) + rtDW->i3) >> 14) - ((int16_T)
      ((rtDW->sigIdx < 0) + rtDW->sigIdx) >> 1);
    if (rtDW->i3 > 32767) {
      rtDW->i3 = 32767;
    } else if (rtDW->i3 < -32768) {
      rtDW->i3 = -32768;
    }

    /* Sum: '<S73>/Sum2' incorporates:
     *  Sum: '<S73>/Sum6'
     *  Sum: '<S74>/Sum6'
     */
    rtDW->i2 = -rtDW->sigIdx - rtDW->i3;
    if (rtDW->i2 > 32767) {
      rtDW->i2 = 32767;
    } else if (rtDW->i2 < -32768) {
      rtDW->i2 = -32768;
    }

    /* MinMax: '<S73>/MinMax1' incorporates:
     *  Sum: '<S74>/Sum6'
     */
    rtDW->Saturation = (int16_T)rtDW->sigIdx;

    /* MinMax: '<S73>/MinMax2' incorporates:
     *  MinMax: '<S73>/MinMax1'
     *  Sum: '<S74>/Sum6'
     */
    rtDW->Saturation1 = (int16_T)rtDW->sigIdx;

    /* MinMax: '<S73>/MinMax1' incorporates:
     *  Sum: '<S73>/Sum6'
     *  Sum: '<S74>/Sum6'
     */
    if (rtDW->sigIdx > rtDW->i3) {
      rtDW->Saturation = (int16_T)rtDW->i3;
    }

    /* MinMax: '<S73>/MinMax2' incorporates:
     *  Sum: '<S73>/Sum6'
     *  Sum: '<S74>/Sum6'
     */
    if (rtDW->sigIdx < rtDW->i3) {
      rtDW->Saturation1 = (int16_T)rtDW->i3;
    }

    /* MinMax: '<S73>/MinMax1' incorporates:
     *  Sum: '<S73>/Sum2'
     */
    if (rtDW->Saturation > rtDW->i2) {
      rtDW->Saturation = (int16_T)rtDW->i2;
    }

    /* MinMax: '<S73>/MinMax2' incorporates:
     *  Sum: '<S73>/Sum2'
     */
    if (rtDW->Saturation1 < rtDW->i2) {
      rtDW->Saturation1 = (int16_T)rtDW->i2;
    }

    /* Sum: '<S73>/Add' incorporates:
     *  MinMax: '<S73>/MinMax1'
     *  MinMax: '<S73>/MinMax2'
     */
    rtDW->i1 = rtDW->Saturation + rtDW->Saturation1;
    if (rtDW->i1 > 32767) {
      rtDW->i1 = 32767;
    } else if (rtDW->i1 < -32768) {
      rtDW->i1 = -32768;
    }

    /* Gain: '<S73>/Gain2' incorporates:
     *  Sum: '<S73>/Add'
     */
    rtDW->Switch2_g = (int16_T)(rtDW->i1 >> 1);

    /* Sum: '<S73>/Add1' incorporates:
     *  Gain: '<S73>/Gain2'
     *  Sum: '<S74>/Sum6'
     */
    rtDW->sigIdx -= rtDW->Switch2_g;
    if (rtDW->sigIdx > 32767) {
      rtDW->sigIdx = 32767;
    } else if (rtDW->sigIdx < -32768) {
      rtDW->sigIdx = -32768;
    }

    /* Gain: '<S73>/Gain4' incorporates:
     *  Sum: '<S73>/Add1'
     */
    rtDW->Gain4_o[0] = (int16_T)((18919 * rtDW->sigIdx) >> 14);

    /* Sum: '<S73>/Add1' incorporates:
     *  Gain: '<S73>/Gain2'
     *  Sum: '<S73>/Sum6'
     */
    rtDW->sigIdx = rtDW->i3 - rtDW->Switch2_g;
    if (rtDW->sigIdx > 32767) {
      rtDW->sigIdx = 32767;
    } else if (rtDW->sigIdx < -32768) {
      rtDW->sigIdx = -32768;
    }

    /* Gain: '<S73>/Gain4' incorporates:
     *  Sum: '<S73>/Add1'
     */
    rtDW->Gain4_o[1] = (int16_T)((18919 * rtDW->sigIdx) >> 14);

    /* Sum: '<S73>/Add1' incorporates:
     *  Gain: '<S73>/Gain2'
     *  Sum: '<S73>/Sum2'
     */
    rtDW->sigIdx = rtDW->i2 - rtDW->Switch2_g;
    if (rtDW->sigIdx > 32767) {
      rtDW->sigIdx = 32767;
    } else if (rtDW->sigIdx < -32768) {
      rtDW->sigIdx = -32768;
    }

    /* Gain: '<S73>/Gain4' incorporates:
     *  Sum: '<S73>/Add1'
     */
    rtDW->Gain4_o[2] = (int16_T)((18919 * rtDW->sigIdx) >> 14);

    /* End of Outputs for SubSystem: '<S7>/Clarke_Park_Transform_Inverse' */
  }

  /* End of If: '<S7>/If2' */

  /* If: '<S8>/If' incorporates:
   *  Constant: '<S1>/z_ctrlTypSel'
   *  Constant: '<S8>/CTRL_COMM2'
   *  Constant: '<S8>/CTRL_COMM3'
   *  Logic: '<S8>/Logical Operator1'
   *  Logic: '<S8>/Logical Operator2'
   *  RelationalOperator: '<S8>/Relational Operator1'
   *  RelationalOperator: '<S8>/Relational Operator2'
   */
  if (rtb_LogicalOperator && (rtP->z_ctrlTypSel == 2)) {
    /* Outputs for IfAction SubSystem: '<S8>/FOC_Method' incorporates:
     *  ActionPort: '<S288>/Action Port'
     */
    /* SignalConversion generated from: '<S288>/V_phaABC_FOC_in' incorporates:
     *  Gain: '<S73>/Gain4'
     *  Merge: '<S8>/Merge'
     */
    rtDW->Saturation = rtDW->Gain4_o[0];
    rtDW->Saturation1 = rtDW->Gain4_o[1];
    rtDW->Switch2_g = rtDW->Gain4_o[2];

    /* End of Outputs for SubSystem: '<S8>/FOC_Method' */
  } else if (rtb_LogicalOperator && (rtP->z_ctrlTypSel == 1)) {
    /* Outputs for IfAction SubSystem: '<S8>/SIN_Method' incorporates:
     *  ActionPort: '<S289>/Action Port'
     */
    /* Switch: '<S290>/Switch_PhaAdv' incorporates:
     *  Constant: '<S290>/a_elecPeriod2'
     *  Constant: '<S290>/b_fieldWeakEna'
     *  Merge: '<S3>/Merge'
     *  Product: '<S291>/Divide2'
     *  Product: '<S291>/Divide3'
     *  Sum: '<S290>/Sum3'
     *  Sum: '<S291>/Sum3'
     */
    if (rtP->b_fieldWeakEna) {
      /* Sum: '<S290>/Sum3' incorporates:
       *  Merge: '<S3>/Merge'
       *  Product: '<S290>/Product2'
       *  SignalConversion generated from: '<S6>/r_fieldWeak '
       */
      rtb_f = (uint16_T)((uint16_T)((uint16_T)(rtDW->OutportBufferForr_fieldWeak
        * rtDW->Switch2_a) << 3) + rtDW->DataTypeConversion2);
      rtb_f = (uint16_T)(rtb_f - (uint32_T)(((int32_T)(rtb_f / 46080U) * 360) <<
        7));
    } else {
      rtb_f = rtDW->DataTypeConversion2;
    }

    /* End of Switch: '<S290>/Switch_PhaAdv' */

    /* PreLookup: '<S289>/a_elecAngle_XA' incorporates:
     *  Switch: '<S290>/Switch_PhaAdv'
     */
    Sum = plook_u8u16_evencka(rtb_f, 0U, 256U, 180U);

    /* Product: '<S289>/Divide2' incorporates:
     *  Interpolation_n-D: '<S289>/r_sin3PhaA_M1'
     *  Interpolation_n-D: '<S289>/r_sin3PhaB_M1'
     *  Interpolation_n-D: '<S289>/r_sin3PhaC_M1'
     *  Merge: '<S8>/Merge'
     *  Switch: '<S8>/Switch2'
     */
    rtDW->Saturation = (int16_T)((rtb_Switch2_j *
      rtConstP.r_sin3PhaA_M1_Table[Sum]) >> 14);
    rtDW->Saturation1 = (int16_T)((rtb_Switch2_j *
      rtConstP.r_sin3PhaB_M1_Table[Sum]) >> 14);
    rtDW->Switch2_g = (int16_T)((rtb_Switch2_j *
      rtConstP.r_sin3PhaC_M1_Table[Sum]) >> 14);

    /* End of Outputs for SubSystem: '<S8>/SIN_Method' */
  } else {
    /* Selector: '<S11>/Selector' incorporates:
     *  Constant: '<S11>/vec_hallToPos'
     */
    rtb_Sum2_ii = rtConstP.vec_hallToPos_Value[Sum];

    /* Outputs for IfAction SubSystem: '<S8>/COM_Method' incorporates:
     *  ActionPort: '<S287>/Action Port'
     */
    /* LookupNDDirect: '<S287>/z_commutMap_M1'
     *
     * About '<S287>/z_commutMap_M1':
     *  2-dimensional Direct Look-Up returning a Column,
     *  which is contiguous for column-major array
     *     Remove protection against out-of-range input in generated code: 'off'
     *   */
    if (rtb_Sum2_ii > 5) {
      rtb_Sum2_ii = 5;
    } else if (rtb_Sum2_ii < 0) {
      rtb_Sum2_ii = 0;
    }

    rtDW->sigIdx = rtb_Sum2_ii * 3;

    /* Product: '<S287>/Divide2' incorporates:
     *  LookupNDDirect: '<S287>/z_commutMap_M1'
     *  Merge: '<S8>/Merge'
     *  Switch: '<S8>/Switch2'
     *
     * About '<S287>/z_commutMap_M1':
     *  2-dimensional Direct Look-Up returning a Column,
     *  which is contiguous for column-major array
     *     Remove protection against out-of-range input in generated code: 'off'
     *   */
    rtDW->Saturation = (int16_T)(rtb_Switch2_j *
      rtConstP.z_commutMap_M1_table[rtDW->sigIdx]);
    rtDW->Saturation1 = (int16_T)(rtConstP.z_commutMap_M1_table[rtDW->sigIdx + 1]
      * rtb_Switch2_j);
    rtDW->Switch2_g = (int16_T)(rtConstP.z_commutMap_M1_table[rtDW->sigIdx + 2] *
      rtb_Switch2_j);

    /* End of Outputs for SubSystem: '<S8>/COM_Method' */
  }

  /* End of If: '<S8>/If' */

  /* Update for UnitDelay: '<S10>/UnitDelay3' */
  rtDW->UnitDelay3_DSTATE_f = rtU->b_hallA;

  /* Update for UnitDelay: '<S10>/UnitDelay1' */
  rtDW->UnitDelay1_DSTATE = rtU->b_hallB;

  /* Update for UnitDelay: '<S10>/UnitDelay2' */
  rtDW->UnitDelay2_DSTATE_k = rtU->b_hallC;

  /* Update for UnitDelay: '<S13>/UnitDelay3' */
  rtDW->UnitDelay3_DSTATE = rtDW->Switch3;

  /* Update for UnitDelay: '<S13>/UnitDelay4' incorporates:
   *  Abs: '<S13>/Abs5'
   */
  rtDW->UnitDelay4_DSTATE_f = Abs5;

  /* Update for UnitDelay: '<S2>/UnitDelay2' incorporates:
   *  UnitDelay: '<S2>/UnitDelay6'
   */
  rtDW->UnitDelay2_DSTATE_g = rtDW->UnitDelay6_DSTATE;

  /* Update for UnitDelay: '<S2>/UnitDelay5' */
  rtDW->UnitDelay5_DSTATE_l = rtb_RelationalOperator4;

  /* Update for UnitDelay: '<S2>/UnitDelay6' */
  rtDW->UnitDelay6_DSTATE = rtb_n_commDeacv;

  /* Update for UnitDelay: '<S8>/UnitDelay4' incorporates:
   *  Switch: '<S8>/Switch2'
   */
  rtDW->UnitDelay4_DSTATE_a = rtb_Switch2_j;

  /* Outport: '<Root>/DC_phaA' incorporates:
   *  DataTypeConversion: '<S8>/Data Type Conversion6'
   *  Merge: '<S8>/Merge'
   */
  rtY->DC_phaA = (int16_T)(rtDW->Saturation >> 4);

  /* Outport: '<Root>/DC_phaB' incorporates:
   *  DataTypeConversion: '<S8>/Data Type Conversion6'
   *  Merge: '<S8>/Merge'
   */
  rtY->DC_phaB = (int16_T)(rtDW->Saturation1 >> 4);

  /* Outport: '<Root>/DC_phaC' incorporates:
   *  DataTypeConversion: '<S8>/Data Type Conversion6'
   *  Merge: '<S8>/Merge'
   */
  rtY->DC_phaC = (int16_T)(rtDW->Switch2_g >> 4);

  /* Outport: '<Root>/n_mot' incorporates:
   *  DataTypeConversion: '<S1>/Data Type Conversion1'
   *  Switch: '<S13>/Switch2'
   */
  rtY->n_mot = (int16_T)(Switch2 >> 4);

  /* Outport: '<Root>/a_elecAngle' incorporates:
   *  DataTypeConversion: '<S1>/Data Type Conversion3'
   *  Merge: '<S3>/Merge'
   */
  rtY->a_elecAngle = (int16_T)(rtDW->DataTypeConversion2 >> 7);

  /* End of Outputs for SubSystem: '<Root>/BLDC_controller' */
}

/* Model initialize function */
void BLDC_controller_initialize(RT_MODEL *const rtM)
{
  P *rtP = ((P *) rtM->defaultParam);
  DW *rtDW = rtM->dwork;

  /* SystemInitialize for Atomic SubSystem: '<Root>/BLDC_controller' */
  /* Start for If: '<S7>/If1' */
  rtDW->If1_ActiveSubsystem = -1;

  /* Start for If: '<S7>/If2' */
  rtDW->If2_ActiveSubsystem = -1;

  /* InitializeConditions for UnitDelay: '<S13>/UnitDelay3' */
  rtDW->UnitDelay3_DSTATE = rtP->z_maxCntRst;

  /* InitializeConditions for UnitDelay: '<S2>/UnitDelay2' */
  rtDW->UnitDelay2_DSTATE_g = true;

  /* SystemInitialize for IfAction SubSystem: '<S13>/Raw_Motor_Speed_Estimation' */
  /* SystemInitialize for SignalConversion generated from: '<S17>/z_counterRawPrev' incorporates:
   *  Outport: '<S17>/z_counter'
   */
  rtDW->z_counterRawPrev = rtP->z_maxCntRst;

  /* End of SystemInitialize for SubSystem: '<S13>/Raw_Motor_Speed_Estimation' */

  /* SystemInitialize for Atomic SubSystem: '<S13>/Counter' */
  Counter_Init(rtP->z_maxCntRst, &rtDW->Counter_c);

  /* End of SystemInitialize for SubSystem: '<S13>/Counter' */

  /* SystemInitialize for IfAction SubSystem: '<S7>/Clarke_Park_Transform_Forward' */
  /* Start for If: '<S61>/If2' */
  rtDW->If2_ActiveSubsystem_a = -1;

  /* End of SystemInitialize for SubSystem: '<S7>/Clarke_Park_Transform_Forward' */

  /* SystemInitialize for Chart: '<S1>/Task_Scheduler' incorporates:
   *  SubSystem: '<S1>/F02_Diagnostics'
   */
  /* SystemInitialize for IfAction SubSystem: '<S4>/Diagnostics_Enabled' */
  /* SystemInitialize for Atomic SubSystem: '<S33>/Debounce_Filter' */
  Debounce_Filter_Init(&rtDW->Merge_a, &rtDW->Debounce_Filter_e);

  /* End of SystemInitialize for SubSystem: '<S33>/Debounce_Filter' */
  /* End of SystemInitialize for SubSystem: '<S4>/Diagnostics_Enabled' */

  /* SystemInitialize for Chart: '<S1>/Task_Scheduler' incorporates:
   *  SubSystem: '<S1>/F03_Control_Mode_Manager'
   */
  F03_Control_Mode_Manager_Init(&rtDW->z_ctrlMod, &rtDW->Merge1,
    &rtDW->F03_Control_Mode_Manager_f);

  /* SystemInitialize for Chart: '<S1>/Task_Scheduler' incorporates:
   *  SubSystem: '<S7>/Motor_Limitations'
   */
  /* Start for If: '<S64>/If1' */
  rtDW->If1_ActiveSubsystem_b = -1;

  /* SystemInitialize for IfAction SubSystem: '<S64>/Motor_Limitations_Enabled' */
  /* Start for SwitchCase: '<S216>/Switch Case' */
  rtDW->SwitchCase_ActiveSubsystem = -1;

  /* SystemInitialize for IfAction SubSystem: '<S216>/Voltage_Mode_Protection' */
  /* SystemInitialize for Atomic SubSystem: '<S220>/I_backCalc_fixdt' */
  I_backCalc_fixdt_Init(65536000, &rtDW->I_backCalc_fixdt_c);

  /* End of SystemInitialize for SubSystem: '<S220>/I_backCalc_fixdt' */

  /* SystemInitialize for Atomic SubSystem: '<S220>/I_backCalc_fixdt1' */
  I_backCalc_fixdt_Init(65536000, &rtDW->I_backCalc_fixdt1);

  /* End of SystemInitialize for SubSystem: '<S220>/I_backCalc_fixdt1' */
  /* End of SystemInitialize for SubSystem: '<S216>/Voltage_Mode_Protection' */

  /* SystemInitialize for IfAction SubSystem: '<S216>/Torque_Mode_Protection' */
  /* InitializeConditions for DiscreteIntegrator: '<S258>/Integrator' */
  rtDW->Integrator_IC_LOADING = 1U;

  /* End of SystemInitialize for SubSystem: '<S216>/Torque_Mode_Protection' */

  /* SystemInitialize for VariantMerge generated from: '<S219>/Vd_max' incorporates:
   *  Outport: '<S216>/Vd_max'
   */
  rtDW->VariantMergeForOutportVd_max = 14400;

  /* SystemInitialize for VariantMerge generated from: '<S219>/Vd_min' incorporates:
   *  Outport: '<S216>/Vd_min'
   */
  rtDW->VariantMergeForOutportVd_min = -14400;

  /* SystemInitialize for VariantMerge generated from: '<S219>/Vq_max' incorporates:
   *  Outport: '<S216>/Vq_max'
   */
  rtDW->VariantMergeForOutportVq_max = 14400;

  /* SystemInitialize for VariantMerge generated from: '<S219>/Vq_min' incorporates:
   *  Outport: '<S216>/Vq_min'
   */
  rtDW->VariantMergeForOutportVq_min = -14400;

  /* SystemInitialize for SignalConversion generated from: '<S216>/id_max' incorporates:
   *  Outport: '<S216>/id_max'
   */
  rtDW->OutportBufferForid_max = 12000;

  /* SystemInitialize for Gain: '<S216>/Gain4' incorporates:
   *  Outport: '<S216>/id_min'
   */
  rtDW->Gain4 = -12000;

  /* SystemInitialize for Product: '<S216>/Divide1' incorporates:
   *  Outport: '<S216>/iq_max'
   */
  rtDW->Divide1_d = 12000;

  /* SystemInitialize for Gain: '<S216>/Gain1' incorporates:
   *  Outport: '<S216>/iq_min'
   */
  rtDW->Gain1 = -12000;

  /* End of SystemInitialize for SubSystem: '<S64>/Motor_Limitations_Enabled' */

  /* SystemInitialize for Chart: '<S1>/Task_Scheduler' incorporates:
   *  SubSystem: '<S7>/FOC'
   */
  FOC_Init(&rtDW->Merge, &rtDW->FOC_h);

  /* End of SystemInitialize for SubSystem: '<Root>/BLDC_controller' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
