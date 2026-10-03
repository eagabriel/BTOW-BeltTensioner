/*
 * File: BLDC_controller.h
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

#ifndef BLDC_controller_h_
#define BLDC_controller_h_
#ifndef BLDC_controller_COMMON_INCLUDES_
#define BLDC_controller_COMMON_INCLUDES_
#include "rtwtypes.h"
#ifndef mcu_model
#include "config.h"
#endif
#endif                                 /* BLDC_controller_COMMON_INCLUDES_ */

/* Model Code Variants */
/**
 * You can use either of the following to alter the value of "normal" variant
 * control variables:
 *
 *  1. -DVC_VARIABLE1=VALUE1 -DVC_VARIABLE2=VALUE2, etc.
 *  2. -DUSE_VARIANT_DEFINES_HEADER="header.h" and within header.h you
 *      specify "#define VC_VARIABLE VALUE1", etc.
 *
 * Variant control variables are the independent variables of variant control
 * expressions specified in a variant block. For example, a Variant Source block
 * may contain "V==1" and V is the variant control variable of this
 * expression. A normal variant control variable is a plain MATLAB variable,
 * i.e. not a Simulink.Parameter.  The default define for a normal variant
 * control variable is the value specified in MATLAB at time of code generation.
 *
 * Alternatively, you can use Simulink.Parameter's as variant control variables to
 * explicitly specify code generation behavior.
 */
#ifdef USE_VARIANT_DEFINES_HEADER
#define VARIANT_DEFINES_HEADER_STR(h)  #h
#define VARIANT_DEFINES_HEADER(h)      VARIANT_DEFINES_HEADER_STR(h)
#include VARIANT_DEFINES_HEADER(USE_VARIANT_DEFINES_HEADER)
#endif                                 /* USE_VARIANT_DEFINES_HEADER */

/*
 * Validate the variant control variables are consistent with the model requirements
 */
#ifndef FeedForwardEnable
#define FeedForwardEnable              0
#endif

#ifndef mcu_model
#define mcu_model                      1
#endif

/* MW_VALIDATE_PREPROCESSOR_VARIANT_CHOICES */
#undef MW_HAVE_ACTIVE_VARIANT_CHOICE
#undef MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#if mcu_model == 0
#ifdef MW_HAVE_ACTIVE_VARIANT_CHOICE
#define MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#else
#define MW_HAVE_ACTIVE_VARIANT_CHOICE
#endif
#endif

#if mcu_model == 1
#ifdef MW_HAVE_ACTIVE_VARIANT_CHOICE
#define MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#else
#define MW_HAVE_ACTIVE_VARIANT_CHOICE
#endif
#endif

/* At most one variant choice can be active for block 'BLDC_controller/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/Variant Subsystem1', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Variant Subsystem' */
#ifdef MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#error "At most one variant choice can be active for block 'BLDC_controller/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1' and others"
#endif

/*At least one variant choice must be active for block 'BLDC_controller/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/Variant Subsystem1', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Variant Subsystem' */
#ifndef MW_HAVE_ACTIVE_VARIANT_CHOICE
#error "At least one variant choice must be active for block 'BLDC_controller/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1' and others"
#endif

#undef MW_HAVE_ACTIVE_VARIANT_CHOICE
#undef MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#if FeedForwardEnable == 0
#ifdef MW_HAVE_ACTIVE_VARIANT_CHOICE
#define MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#else
#define MW_HAVE_ACTIVE_VARIANT_CHOICE
#endif
#endif

#if FeedForwardEnable == 1
#ifdef MW_HAVE_ACTIVE_VARIANT_CHOICE
#define MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#else
#define MW_HAVE_ACTIVE_VARIANT_CHOICE
#endif
#endif

/* At most one variant choice can be active for block 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant' */
#ifdef MW_HAVE_MORE_THAN_ONE_ACTIVE_CHOICE
#error "At most one variant choice can be active for block 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant' and others"
#endif

/*At least one variant choice must be active for block 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant', 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant' */
#ifndef MW_HAVE_ACTIVE_VARIANT_CHOICE
#error "At least one variant choice must be active for block 'BLDC_controller/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant' and others"
#endif

/* MW_END_VALIDATE_PREPROCESSOR_VARIANT_CHOICES */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetRootDWork
#define rtmGetRootDWork(rtm)           ((rtm)->dwork)
#endif

#ifndef rtmSetRootDWork
#define rtmSetRootDWork(rtm, val)      ((rtm)->dwork = (val))
#endif

#ifndef rtmGetU
#define rtmGetU(rtm)                   ((rtm)->inputs)
#endif

#ifndef rtmSetU
#define rtmSetU(rtm, val)              ((rtm)->inputs = (val))
#endif

#ifndef rtmGetY
#define rtmGetY(rtm)                   ((rtm)->outputs)
#endif

#ifndef rtmSetY
#define rtmSetY(rtm, val)              ((rtm)->outputs = (val))
#endif

#define BLDC_controller_M              (rtM)

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (default storage) for system '<S13>/Counter' */
typedef struct {
  int16_T UnitDelay_DSTATE;            /* '<S18>/UnitDelay' */
} DW_Counter;

/* Block signals and states (default storage) for system '<S66>/Low_Pass_Filter' */
typedef struct {
  int32_T UnitDelay1_DSTATE[2];        /* '<S72>/UnitDelay1' */
} DW_Low_Pass_Filter;

/* Block signals and states (default storage) for system '<S38>/Counter' */
typedef struct {
  uint16_T UnitDelay_DSTATE;           /* '<S43>/UnitDelay' */
} DW_Counter_d;

/* Block signals and states (default storage) for system '<S37>/Counter' */
typedef struct {
  uint32_T UnitDelay_DSTATE;           /* '<S41>/UnitDelay' */
} DW_Counter_e;

/* Block signals and states (default storage) for system '<S34>/either_edge' */
typedef struct {
  boolean_T UnitDelay_DSTATE;          /* '<S39>/UnitDelay' */
} DW_either_edge;

/* Block signals and states (default storage) for system '<S33>/Debounce_Filter' */
typedef struct {
  DW_either_edge either_edge_n;        /* '<S34>/either_edge' */
  DW_Counter_e Counter_ni;             /* '<S37>/Counter' */
  DW_Counter_d Counter_e2;             /* '<S38>/Counter' */
  boolean_T UnitDelay_DSTATE;          /* '<S34>/UnitDelay' */
} DW_Debounce_Filter;

/* Block signals and states (default storage) for system '<S1>/F03_Control_Mode_Manager' */
typedef struct {
  int32_T UnitDelay_DSTATE;            /* '<S56>/UnitDelay' */
  int8_T If2_ActiveSubsystem;          /* '<S46>/If2' */
  uint8_T is_active_c2_BLDC_controller;/* '<S5>/F03_02_Control_Mode_Manager' */
  uint8_T is_c2_BLDC_controller;       /* '<S5>/F03_02_Control_Mode_Manager' */
  uint8_T is_ACTIVE;                   /* '<S5>/F03_02_Control_Mode_Manager' */
  boolean_T UnitDelay_DSTATE_c;        /* '<S55>/UnitDelay' */
} DW_F03_Control_Mode_Manager;

/* Block signals and states (default storage) for system '<S220>/I_backCalc_fixdt' */
typedef struct {
  int32_T UnitDelay_DSTATE;            /* '<S281>/UnitDelay' */
  int32_T UnitDelay_DSTATE_a;          /* '<S283>/UnitDelay' */
} DW_I_backCalc_fixdt;

/* Block signals and states (default storage) for system '<S153>/PI_clamp_fixdtFF' */
typedef struct {
  int32_T ResettableDelay_DSTATE;      /* '<S213>/Resettable Delay' */
  boolean_T UnitDelay1_DSTATE;         /* '<S211>/UnitDelay1' */
  boolean_T icLoad;                    /* '<S213>/Resettable Delay' */
} DW_PI_clamp_fixdtFF;

/* Block signals and states (default storage) for system '<S77>/PI_clamp_fixdt' */
typedef struct {
  int32_T ResettableDelay_DSTATE;      /* '<S83>/Resettable Delay' */
  boolean_T UnitDelay1_DSTATE;         /* '<S81>/UnitDelay1' */
  boolean_T icLoad;                    /* '<S83>/Resettable Delay' */
} DW_PI_clamp_fixdt;

/* Block signals and states (default storage) for system '<S88>/PI_clamp_fixdtFF' */
typedef struct {
  int32_T ResettableDelay_DSTATE;      /* '<S148>/Resettable Delay' */
  boolean_T UnitDelay1_DSTATE;         /* '<S146>/UnitDelay1' */
  boolean_T icLoad;                    /* '<S148>/Resettable Delay' */
} DW_PI_clamp_fixdtFF_k;

/* Block signals and states (default storage) for system '<S7>/FOC' */
typedef struct {

#if FeedForwardEnable == 1

  DW_PI_clamp_fixdtFF_k PI_clamp_fixdtFF_ci;/* '<S88>/PI_clamp_fixdtFF' */

#endif

  DW_PI_clamp_fixdt PI_clamp_fixdt_j;  /* '<S77>/PI_clamp_fixdt' */

#if FeedForwardEnable == 1

  DW_PI_clamp_fixdtFF PI_clamp_fixdtFF_c2;/* '<S153>/PI_clamp_fixdtFF' */

#endif

  int16_T constant2;                   /* '<S79>/constant2' */

#if FeedForwardEnable == 0

  int16_T Integrator_DSTATE;           /* '<S127>/Integrator' */

#endif

#if FeedForwardEnable == 0

  int16_T Integrator_DSTATE_a;         /* '<S192>/Integrator' */

#endif

  int8_T If1_ActiveSubsystem;          /* '<S63>/If1' */
  int8_T SwitchCase_ActiveSubsystem;   /* '<S75>/Switch Case' */
  int8_T If1_ActiveSubsystem_a;        /* '<S75>/If1' */

#if FeedForwardEnable == 0

  uint8_T Integrator_IC_LOADING;       /* '<S127>/Integrator' */

#endif

} DW_FOC;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  DW_FOC FOC_h;                        /* '<S7>/FOC' */
  DW_I_backCalc_fixdt I_backCalc_fixdt1;/* '<S220>/I_backCalc_fixdt1' */
  DW_I_backCalc_fixdt I_backCalc_fixdt_c;/* '<S220>/I_backCalc_fixdt' */
  DW_F03_Control_Mode_Manager F03_Control_Mode_Manager_f;/* '<S1>/F03_Control_Mode_Manager' */
  DW_either_edge either_edge_j;        /* '<S33>/either_edge' */
  DW_Debounce_Filter Debounce_Filter_e;/* '<S33>/Debounce_Filter' */
  DW_Low_Pass_Filter Low_Pass_Filter_e;/* '<S66>/Low_Pass_Filter' */
  DW_Counter Counter_c;                /* '<S13>/Counter' */
  int64_T i;
  int32_T Divide1;                     /* '<S217>/Divide1' */
  int32_T sigIdx;
  int32_T i1;
  int32_T i2;
  int32_T i3;
  int16_T Gain4_o[3];                  /* '<S73>/Gain4' */
  int16_T DataTypeConversion[2];       /* '<S72>/Data Type Conversion' */
  int16_T TmpSignalConversionAtLow_Pa[2];
  int16_T Merge;                       /* '<S75>/Merge' */
  int16_T VariantMergeForOutportVq_Trq;
  int16_T Divide1_d;                   /* '<S216>/Divide1' */
  int16_T Gain1;                       /* '<S216>/Gain1' */
  int16_T Gain4;                       /* '<S216>/Gain4' */
  int16_T OutportBufferForid_max;
  int16_T VariantMergeForOutportVd_max;
  int16_T VariantMergeForOutportVd_min;
  int16_T VariantMergeForOutportVq_max;
  int16_T VariantMergeForOutportVq_min;
  int16_T Switch2_m;                   /* '<S266>/Switch2' */
  int16_T Switch2_p;                   /* '<S286>/Switch2' */
  int16_T Switch2_b;                   /* '<S284>/Switch2' */
  int16_T OutportBufferForr_fieldWeak;
  int16_T Divide3;                     /* '<S58>/Divide3' */
  int16_T Merge1;                      /* '<S46>/Merge1' */
  int16_T Abs1;                        /* '<S5>/Abs1' */
  int16_T OutportBufferForiqAbs;
  int16_T Abs5_d;                      /* '<S66>/Abs5' */
  int16_T OutportBufferForid_f;
  int16_T Divide11;                    /* '<S17>/Divide11' */
  int16_T r_sin_M1_1;                  /* '<S68>/r_sin_M1' */
  int16_T r_cos_M1_1;                  /* '<S68>/r_cos_M1' */
  int16_T z_counterRawPrev;            /* '<S17>/z_counterRawPrev' */
  int16_T UnitDelay3_DSTATE;           /* '<S13>/UnitDelay3' */
  int16_T UnitDelay4_DSTATE;           /* '<S17>/UnitDelay4' */
  int16_T UnitDelay2_DSTATE;           /* '<S17>/UnitDelay2' */
  int16_T UnitDelay3_DSTATE_k;         /* '<S17>/UnitDelay3' */
  int16_T UnitDelay5_DSTATE;           /* '<S17>/UnitDelay5' */
  int16_T UnitDelay4_DSTATE_f;         /* '<S13>/UnitDelay4' */
  int16_T UnitDelay4_DSTATE_a;         /* '<S8>/UnitDelay4' */
  int16_T Integrator_DSTATE;           /* '<S258>/Integrator' */
  int16_T Switch3;                     /* '<S17>/Switch3' */
  int16_T Saturation;                  /* '<S1>/Saturation' */
  int16_T Saturation1;                 /* '<S1>/Saturation1' */
  int16_T Switch2_g;                   /* '<S14>/Switch2' */
  uint16_T DataTypeConversion2;        /* '<S22>/Data Type Conversion2' */
  int8_T Switch2_a;                    /* '<S12>/Switch2' */
  int8_T UnitDelay2_DSTATE_o;          /* '<S12>/UnitDelay2' */
  int8_T If1_ActiveSubsystem;          /* '<S7>/If1' */
  int8_T If2_ActiveSubsystem;          /* '<S7>/If2' */
  int8_T If1_ActiveSubsystem_b;        /* '<S64>/If1' */
  int8_T SwitchCase_ActiveSubsystem;   /* '<S216>/Switch Case' */
  int8_T If2_ActiveSubsystem_a;        /* '<S61>/If2' */
  uint8_T z_ctrlMod;                   /* '<S5>/F03_02_Control_Mode_Manager' */
  uint8_T Switch1;                     /* '<S33>/Switch1' */
  uint8_T UnitDelay3_DSTATE_f;         /* '<S10>/UnitDelay3' */
  uint8_T UnitDelay1_DSTATE;           /* '<S10>/UnitDelay1' */
  uint8_T UnitDelay2_DSTATE_k;         /* '<S10>/UnitDelay2' */
  uint8_T Integrator_IC_LOADING;       /* '<S258>/Integrator' */
  boolean_T OutportBufferForb_errFlag;
  boolean_T Merge_a;                   /* '<S34>/Merge' */
  boolean_T dz_cntTrnsDet;             /* '<S17>/dz_cntTrnsDet' */
  boolean_T UnitDelay2_DSTATE_g;       /* '<S2>/UnitDelay2' */
  boolean_T UnitDelay5_DSTATE_l;       /* '<S2>/UnitDelay5' */
  boolean_T UnitDelay6_DSTATE;         /* '<S2>/UnitDelay6' */
  boolean_T UnitDelay1_DSTATE_g;       /* '<S17>/UnitDelay1' */
  boolean_T n_commDeacv_Mode;          /* '<S13>/n_commDeacv' */
  boolean_T dz_cntTrnsDet_Mode;        /* '<S17>/dz_cntTrnsDet' */
} DW;

/* Constant parameters (default storage) */
typedef struct {
  /* Computed Parameter: r_sin_M1_1_Table
   * Referenced by: '<S68>/r_sin_M1'
   */
  int16_T r_sin_M1_1_Table[181];

  /* Computed Parameter: r_cos_M1_1_Table
   * Referenced by: '<S68>/r_cos_M1'
   */
  int16_T r_cos_M1_1_Table[181];

  /* Computed Parameter: r_sin3PhaA_M1_Table
   * Referenced by: '<S289>/r_sin3PhaA_M1'
   */
  int16_T r_sin3PhaA_M1_Table[181];

  /* Computed Parameter: r_sin3PhaB_M1_Table
   * Referenced by: '<S289>/r_sin3PhaB_M1'
   */
  int16_T r_sin3PhaB_M1_Table[181];

  /* Computed Parameter: r_sin3PhaC_M1_Table
   * Referenced by: '<S289>/r_sin3PhaC_M1'
   */
  int16_T r_sin3PhaC_M1_Table[181];

#if mcu_model == 1

  /* Computed Parameter: GD32_Vq_max_M1_Table
   * Referenced by: '<S279>/GD32_Vq_max_M1'
   */
  int16_T GD32_Vq_max_M1_Table[82];

#ifndef CONSTP_VARIANT_EXISTS
#define CONSTP_VARIANT_EXISTS
#endif
#endif

#if mcu_model == 0

  /* Computed Parameter: STM32_Vq_max_M1_Table
   * Referenced by: '<S280>/STM32_Vq_max_M1'
   */
  int16_T STM32_Vq_max_M1_Table[32];

#ifndef CONSTP_VARIANT_EXISTS
#define CONSTP_VARIANT_EXISTS
#endif
#endif

  /* Computed Parameter: iq_maxSca_M1_Table
   * Referenced by: '<S216>/iq_maxSca_M1'
   */
  uint16_T iq_maxSca_M1_Table[100];

  /* Computed Parameter: z_commutMap_M1_table
   * Referenced by: '<S287>/z_commutMap_M1'
   */
  int8_T z_commutMap_M1_table[18];

  /* Computed Parameter: vec_hallToPos_Value
   * Referenced by: '<S11>/vec_hallToPos'
   */
  int8_T vec_hallToPos_Value[8];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  boolean_T b_motEna;                  /* '<Root>/b_motEna' */
  uint8_T z_ctrlModReq;                /* '<Root>/z_ctrlModReq' */
  int16_T r_inpTgt;                    /* '<Root>/r_inpTgt' */
  uint8_T b_hallA;                     /* '<Root>/b_hallA ' */
  uint8_T b_hallB;                     /* '<Root>/b_hallB' */
  uint8_T b_hallC;                     /* '<Root>/b_hallC' */
  int16_T i_phaAB;                     /* '<Root>/i_phaAB' */
  int16_T i_phaBC;                     /* '<Root>/i_phaBC' */
  int16_T i_DCLink;                    /* '<Root>/i_DCLink' */
  uint16_T a_encoderCNT;               /* '<Root>/a_encoderCNT' */
  int16_T a_mechangle;                 /* '<Root>/a_mechangle' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  int16_T DC_phaA;                     /* '<Root>/DC_phaA' */
  int16_T DC_phaB;                     /* '<Root>/DC_phaB' */
  int16_T DC_phaC;                     /* '<Root>/DC_phaC' */
  uint8_T z_errCode;                   /* '<Root>/z_errCode' */
  int16_T n_mot;                       /* '<Root>/n_mot' */
  int16_T a_elecAngle;                 /* '<Root>/a_elecAngle' */
  int16_T iq;                          /* '<Root>/iq' */
  int16_T id;                          /* '<Root>/id' */
} ExtY;

/* Parameters (default storage) */
struct P_ {
  int32_T dV_openRate;                 /* Variable: dV_openRate
                                        * Referenced by: '<S50>/dV_openRate'
                                        */
  uint32_T a_fcpr;                     /* Variable: a_fcpr
                                        * Referenced by: '<S15>/Constant3'
                                        */
  uint32_T t_errDequal;                /* Variable: t_errDequal
                                        * Referenced by: '<S33>/t_errDequal'
                                        */
  int16_T dz_cntTrnsDetHi;             /* Variable: dz_cntTrnsDetHi
                                        * Referenced by: '<S17>/dz_cntTrnsDet'
                                        */
  int16_T dz_cntTrnsDetLo;             /* Variable: dz_cntTrnsDetLo
                                        * Referenced by: '<S17>/dz_cntTrnsDet'
                                        */
  int16_T n_cruiseMotTgt;              /* Variable: n_cruiseMotTgt
                                        * Referenced by: '<S77>/n_cruiseMotTgt'
                                        */
  int16_T z_maxCntRst;                 /* Variable: z_maxCntRst
                                        * Referenced by:
                                        *   '<S13>/Counter'
                                        *   '<S13>/z_maxCntRst'
                                        *   '<S13>/z_maxCntRst2'
                                        *   '<S13>/UnitDelay3'
                                        *   '<S17>/z_counter'
                                        */

#if FeedForwardEnable == 1

  int16_T ff_gain;                     /* Variable: ff_gain
                                        * Referenced by:
                                        *   '<S78>/Input'
                                        *   '<S79>/Input'
                                        */

#endif

  int16_T a_phaAdvMax;                 /* Variable: a_phaAdvMax
                                        * Referenced by: '<S58>/a_phaAdvMax'
                                        */
  int16_T i_max;                       /* Variable: i_max
                                        * Referenced by:
                                        *   '<S49>/i_max'
                                        *   '<S216>/i_max'
                                        */
  int16_T id_fieldWeakMax;             /* Variable: id_fieldWeakMax
                                        * Referenced by: '<S58>/id_fieldWeakMax'
                                        */
  int16_T n_commAcvLo;                 /* Variable: n_commAcvLo
                                        * Referenced by: '<S13>/n_commDeacv'
                                        */
  int16_T n_commDeacvHi;               /* Variable: n_commDeacvHi
                                        * Referenced by: '<S13>/n_commDeacv'
                                        */
  int16_T n_fieldWeakAuthHi;           /* Variable: n_fieldWeakAuthHi
                                        * Referenced by: '<S58>/n_fieldWeakAuthHi'
                                        */
  int16_T n_fieldWeakAuthLo;           /* Variable: n_fieldWeakAuthLo
                                        * Referenced by: '<S58>/n_fieldWeakAuthLo'
                                        */
  int16_T n_max;                       /* Variable: n_max
                                        * Referenced by:
                                        *   '<S49>/n_max'
                                        *   '<S216>/n_max'
                                        */
  int16_T n_stdStillDet;               /* Variable: n_stdStillDet
                                        * Referenced by: '<S13>/n_stdStillDet'
                                        */
  int16_T r_errInpTgtThres;            /* Variable: r_errInpTgtThres
                                        * Referenced by: '<S33>/r_errInpTgtThres'
                                        */
  int16_T r_fieldWeakHi;               /* Variable: r_fieldWeakHi
                                        * Referenced by: '<S58>/r_fieldWeakHi'
                                        */
  int16_T r_fieldWeakLo;               /* Variable: r_fieldWeakLo
                                        * Referenced by: '<S58>/r_fieldWeakLo'
                                        */
  uint16_T cf_idKp;                    /* Variable: cf_idKp
                                        * Referenced by: '<S79>/cf_idKp2'
                                        */
  uint16_T cf_iqKp;                    /* Variable: cf_iqKp
                                        * Referenced by: '<S78>/cf_iqKp1'
                                        */
  uint16_T cf_nKp;                     /* Variable: cf_nKp
                                        * Referenced by: '<S77>/cf_nKp'
                                        */
  uint16_T cf_currFilt;                /* Variable: cf_currFilt
                                        * Referenced by: '<S66>/cf_currFilt'
                                        */
  uint16_T cf_idKi;                    /* Variable: cf_idKi
                                        * Referenced by: '<S79>/cf_idKi2'
                                        */
  uint16_T cf_iqKi;                    /* Variable: cf_iqKi
                                        * Referenced by: '<S78>/cf_iqKi1'
                                        */
  uint16_T cf_iqKiLimProt;             /* Variable: cf_iqKiLimProt
                                        * Referenced by:
                                        *   '<S217>/cf_iqKiLimProt'
                                        *   '<S220>/cf_iqKiLimProt'
                                        */
  uint16_T cf_nKi;                     /* Variable: cf_nKi
                                        * Referenced by: '<S77>/cf_nKi'
                                        */
  uint16_T cf_nKiLimProt;              /* Variable: cf_nKiLimProt
                                        * Referenced by:
                                        *   '<S218>/cf_nKiLimProt'
                                        *   '<S220>/cf_nKiLimProt'
                                        */
  uint16_T a_cpr;                      /* Variable: a_cpr
                                        * Referenced by: '<S15>/Constant'
                                        */
  uint16_T cf_speedCoef;               /* Variable: cf_speedCoef
                                        * Referenced by: '<S17>/cf_speedCoef'
                                        */
  uint16_T n_polePairs;                /* Variable: n_polePairs
                                        * Referenced by: '<S15>/n_polePairs1'
                                        */
  uint16_T t_errQual;                  /* Variable: t_errQual
                                        * Referenced by: '<S33>/t_errQual'
                                        */
  boolean_T b_angleMeasEna;            /* Variable: b_angleMeasEna
                                        * Referenced by:
                                        *   '<S3>/b_angleMeasEna'
                                        *   '<S13>/b_angleMeasEna'
                                        */
  boolean_T b_cruiseCtrlEna;           /* Variable: b_cruiseCtrlEna
                                        * Referenced by: '<S1>/b_cruiseCtrlEna'
                                        */
  boolean_T b_diagEna;                 /* Variable: b_diagEna
                                        * Referenced by: '<S4>/b_diagEna'
                                        */
  boolean_T b_fieldWeakEna;            /* Variable: b_fieldWeakEna
                                        * Referenced by:
                                        *   '<S6>/b_fieldWeakEna'
                                        *   '<S290>/b_fieldWeakEna'
                                        */
  uint8_T z_ctrlTypSel;                /* Variable: z_ctrlTypSel
                                        * Referenced by: '<S1>/z_ctrlTypSel'
                                        */
  uint8_T z_selPhaCurMeasABC;          /* Variable: z_selPhaCurMeasABC
                                        * Referenced by: '<S65>/z_selPhaCurMeasABC'
                                        */
};

/* Parameters (default storage) */
typedef struct P_ P;

/* Real-time Model Data Structure */
struct tag_RTM {
    P *defaultParam;
ExtU *inputs;
  ExtY *outputs;
  DW *dwork;
};

/* Block parameters (default storage) */
//extern P rtP;
/* Constant parameters (default storage) */
extern const ConstP rtConstP;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern uint16_T cf_KbLimProt;          /* Variable: cf_KbLimProt
                                        * Referenced by:
                                        *   '<S220>/cf_KbLimProt'
                                        *   '<S250>/Kb'
                                        */

/* Model entry point functions */
extern void BLDC_controller_initialize(RT_MODEL *const rtM);
extern void BLDC_controller_step(RT_MODEL *const rtM);

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S13>/Scope2' : Unused code path elimination
 * Block '<S14>/Scope' : Unused code path elimination
 * Block '<S15>/MechElec' : Unused code path elimination
 * Block '<S22>/Data Type Duplicate' : Unused code path elimination
 * Block '<S57>/Data Type Duplicate' : Unused code path elimination
 * Block '<S57>/Data Type Propagation' : Unused code path elimination
 * Block '<S59>/Data Type Duplicate' : Unused code path elimination
 * Block '<S59>/Data Type Propagation' : Unused code path elimination
 * Block '<S60>/Data Type Duplicate' : Unused code path elimination
 * Block '<S60>/Data Type Propagation' : Unused code path elimination
 * Block '<S65>/Scope' : Unused code path elimination
 * Block '<S61>/Scope' : Unused code path elimination
 * Block '<S62>/Scope' : Unused code path elimination
 * Block '<S78>/FOC TQ PI' : Unused code path elimination
 * Block '<S135>/Data Type Duplicate' : Unused code path elimination
 * Block '<S135>/Data Type Propagation' : Unused code path elimination
 * Block '<S86>/Data Type Duplicate' : Unused code path elimination
 * Block '<S86>/Data Type Propagation' : Unused code path elimination
 * Block '<S200>/Data Type Duplicate' : Unused code path elimination
 * Block '<S200>/Data Type Propagation' : Unused code path elimination
 * Block '<S151>/Data Type Duplicate' : Unused code path elimination
 * Block '<S151>/Data Type Propagation' : Unused code path elimination
 * Block '<S215>/Data Type Duplicate' : Unused code path elimination
 * Block '<S215>/Data Type Propagation' : Unused code path elimination
 * Block '<S63>/Scope' : Unused code path elimination
 * Block '<S216>/Scope' : Unused code path elimination
 * Block '<S221>/Data Type Duplicate' : Unused code path elimination
 * Block '<S221>/Data Type Propagation' : Unused code path elimination
 * Block '<S266>/Data Type Duplicate' : Unused code path elimination
 * Block '<S266>/Data Type Propagation' : Unused code path elimination
 * Block '<S284>/Data Type Duplicate' : Unused code path elimination
 * Block '<S284>/Data Type Propagation' : Unused code path elimination
 * Block '<S286>/Data Type Duplicate' : Unused code path elimination
 * Block '<S286>/Data Type Propagation' : Unused code path elimination
 * Block '<S290>/Scope' : Unused code path elimination
 * Block '<S2>/Data Type Conversion' : Eliminate redundant data type conversion
 * Block '<S22>/Data Type Conversion' : Eliminate redundant data type conversion
 * Block '<S31>/Data Type Conversion' : Eliminate redundant data type conversion
 * Block '<S78>/Signal Conversion2' : Eliminate redundant signal conversion block
 * Block '<S22>/Constant' : Unused code path elimination
 * Block '<S22>/Constant1' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('BLDCmotor_FOC_R2017b_fixdt/BLDC_controller')    - opens subsystem BLDCmotor_FOC_R2017b_fixdt/BLDC_controller
 * hilite_system('BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'BLDCmotor_FOC_R2017b_fixdt'
 * '<S1>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller'
 * '<S2>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/Call_Scheduler'
 * '<S3>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations'
 * '<S4>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics'
 * '<S5>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager'
 * '<S6>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F04_Field_Weakening'
 * '<S7>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control'
 * '<S8>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management'
 * '<S9>'   : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/Task_Scheduler'
 * '<S10>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_01_Edge_Detector'
 * '<S11>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_02_Position_Calculation'
 * '<S12>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_03_Direction_Detection'
 * '<S13>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_04_Speed_Estimation'
 * '<S14>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_05_Electrical_Angle_Estimation'
 * '<S15>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement'
 * '<S16>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_04_Speed_Estimation/Counter'
 * '<S17>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_04_Speed_Estimation/Raw_Motor_Speed_Estimation'
 * '<S18>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_04_Speed_Estimation/Counter/rst_Delay'
 * '<S19>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Mechanical to Electrical Position'
 * '<S20>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder'
 * '<S21>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Mechanical to Electrical Position/MechToElec'
 * '<S22>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Mechanical to Electrical Position/MechToElec/fixed-point'
 * '<S23>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Mechanical to Electrical Position/MechToElec/fixed-point/Variant Subsystem'
 * '<S24>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Mechanical to Electrical Position/MechToElec/fixed-point/Variant Subsystem/Input Port'
 * '<S25>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/DT_Handle'
 * '<S26>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/PositionNoReset'
 * '<S27>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/PositionResetAtIndex'
 * '<S28>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/Variant Subsystem'
 * '<S29>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/DT_Handle/fixed-point'
 * '<S30>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/PositionNoReset/Variant Subsystem'
 * '<S31>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/PositionNoReset/Variant Subsystem/Input Port'
 * '<S32>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F01_Estimations/F01_06_Electrical_Angle_Measurement/Quadrature Decoder/Variant Subsystem/Input Port'
 * '<S33>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled'
 * '<S34>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter'
 * '<S35>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/either_edge'
 * '<S36>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Default'
 * '<S37>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Dequalification'
 * '<S38>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Qualification'
 * '<S39>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/either_edge'
 * '<S40>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Dequalification/Counter'
 * '<S41>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Dequalification/Counter/rst_Delay'
 * '<S42>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Qualification/Counter'
 * '<S43>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F02_Diagnostics/Diagnostics_Enabled/Debounce_Filter/Qualification/Counter/rst_Delay'
 * '<S44>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_01_Mode_Transition_Calculation'
 * '<S45>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_02_Control_Mode_Manager'
 * '<S46>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis'
 * '<S47>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Default_Control_Type'
 * '<S48>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Default_Mode'
 * '<S49>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type'
 * '<S50>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Open_Mode'
 * '<S51>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1'
 * '<S52>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1/GD32F103'
 * '<S53>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/FOC_Control_Type/Variant Subsystem1/STM32F103'
 * '<S54>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Open_Mode/Rate_Limiter'
 * '<S55>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Open_Mode/rising_edge_init'
 * '<S56>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Open_Mode/Rate_Limiter/Delay_Init1'
 * '<S57>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F03_Control_Mode_Manager/F03_03_Input_Target_Synthesis/Open_Mode/Rate_Limiter/Saturation Dynamic'
 * '<S58>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F04_Field_Weakening/Field_Weakening_Enabled'
 * '<S59>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F04_Field_Weakening/Field_Weakening_Enabled/Saturation Dynamic'
 * '<S60>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F04_Field_Weakening/Field_Weakening_Enabled/Saturation Dynamic1'
 * '<S61>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward'
 * '<S62>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Inverse'
 * '<S63>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC'
 * '<S64>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations'
 * '<S65>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Clarke_Transform'
 * '<S66>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Current_Filtering'
 * '<S67>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Park_Transform'
 * '<S68>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Sine_Cosine_Approximation'
 * '<S69>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Clarke_Transform/Clarke_PhasesAB'
 * '<S70>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Clarke_Transform/Clarke_PhasesAC'
 * '<S71>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Clarke_Transform/Clarke_PhasesBC'
 * '<S72>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Forward/Current_Filtering/Low_Pass_Filter'
 * '<S73>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Inverse/Inv_Clarke_Transform'
 * '<S74>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Clarke_Park_Transform_Inverse/Inv_Park_Transform'
 * '<S75>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled'
 * '<S76>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Open_Mode'
 * '<S77>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Speed_Mode'
 * '<S78>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode'
 * '<S79>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation'
 * '<S80>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Voltage_Mode'
 * '<S81>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Speed_Mode/PI_clamp_fixdt'
 * '<S82>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Speed_Mode/PI_clamp_fixdt/Clamping_circuit'
 * '<S83>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Speed_Mode/PI_clamp_fixdt/Integrator'
 * '<S84>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Speed_Mode/PI_clamp_fixdt/Saturation_hit'
 * '<S85>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant'
 * '<S86>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Saturation Dynamic1'
 * '<S87>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff'
 * '<S88>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFon'
 * '<S89>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller'
 * '<S90>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Anti-windup'
 * '<S91>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/D Gain'
 * '<S92>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/External Derivative'
 * '<S93>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Filter'
 * '<S94>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Filter ICs'
 * '<S95>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/I Gain'
 * '<S96>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Ideal P Gain'
 * '<S97>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Ideal P Gain Fdbk'
 * '<S98>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Integrator'
 * '<S99>'  : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Integrator ICs'
 * '<S100>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/N Copy'
 * '<S101>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/N Gain'
 * '<S102>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/P Copy'
 * '<S103>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Parallel P Gain'
 * '<S104>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Reset Signal'
 * '<S105>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Saturation'
 * '<S106>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Saturation Fdbk'
 * '<S107>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Sum'
 * '<S108>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Sum Fdbk'
 * '<S109>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tracking Mode'
 * '<S110>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tracking Mode Sum'
 * '<S111>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tsamp - Integral'
 * '<S112>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tsamp - Ngain'
 * '<S113>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/postSat Signal'
 * '<S114>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/preInt Signal'
 * '<S115>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/preSat Signal'
 * '<S116>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel'
 * '<S117>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S118>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/External'
 * '<S119>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/External/Dead Zone Dynamic'
 * '<S120>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/D Gain/Disabled'
 * '<S121>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/External Derivative/Disabled'
 * '<S122>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Filter/Disabled'
 * '<S123>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Filter ICs/Disabled'
 * '<S124>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/I Gain/External Parameters'
 * '<S125>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Ideal P Gain/Passthrough'
 * '<S126>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Ideal P Gain Fdbk/Disabled'
 * '<S127>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Integrator/Discrete'
 * '<S128>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Integrator ICs/External IC'
 * '<S129>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/N Copy/Disabled wSignal Specification'
 * '<S130>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/N Gain/Disabled'
 * '<S131>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/P Copy/Disabled'
 * '<S132>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Parallel P Gain/External Parameters'
 * '<S133>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Reset Signal/Disabled'
 * '<S134>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Saturation/External'
 * '<S135>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Saturation/External/Saturation Dynamic'
 * '<S136>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Saturation Fdbk/Disabled'
 * '<S137>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Sum/Sum_PI'
 * '<S138>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Sum Fdbk/Disabled'
 * '<S139>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tracking Mode/Disabled'
 * '<S140>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tracking Mode Sum/Passthrough'
 * '<S141>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tsamp - Integral/Passthrough'
 * '<S142>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/Tsamp - Ngain/Passthrough'
 * '<S143>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/postSat Signal/Forward_Path'
 * '<S144>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/preInt Signal/Internal PreInt'
 * '<S145>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFoff/PI Controller/preSat Signal/Forward_Path'
 * '<S146>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFon/PI_clamp_fixdtFF'
 * '<S147>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFon/PI_clamp_fixdtFF/Clamping_circuit'
 * '<S148>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFon/PI_clamp_fixdtFF/Integrator'
 * '<S149>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Torque_Mode/Pi Variant/FFon/PI_clamp_fixdtFF/Saturation_hit'
 * '<S150>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant'
 * '<S151>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Saturation Dynamic'
 * '<S152>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff'
 * '<S153>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFon'
 * '<S154>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller'
 * '<S155>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Anti-windup'
 * '<S156>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/D Gain'
 * '<S157>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/External Derivative'
 * '<S158>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Filter'
 * '<S159>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Filter ICs'
 * '<S160>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/I Gain'
 * '<S161>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Ideal P Gain'
 * '<S162>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Ideal P Gain Fdbk'
 * '<S163>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Integrator'
 * '<S164>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Integrator ICs'
 * '<S165>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/N Copy'
 * '<S166>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/N Gain'
 * '<S167>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/P Copy'
 * '<S168>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Parallel P Gain'
 * '<S169>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Reset Signal'
 * '<S170>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Saturation'
 * '<S171>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Saturation Fdbk'
 * '<S172>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Sum'
 * '<S173>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Sum Fdbk'
 * '<S174>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tracking Mode'
 * '<S175>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tracking Mode Sum'
 * '<S176>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tsamp - Integral'
 * '<S177>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tsamp - Ngain'
 * '<S178>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/postSat Signal'
 * '<S179>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/preInt Signal'
 * '<S180>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/preSat Signal'
 * '<S181>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel'
 * '<S182>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S183>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/External'
 * '<S184>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/External/Dead Zone Dynamic'
 * '<S185>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/D Gain/Disabled'
 * '<S186>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/External Derivative/Disabled'
 * '<S187>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Filter/Disabled'
 * '<S188>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Filter ICs/Disabled'
 * '<S189>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/I Gain/External Parameters'
 * '<S190>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Ideal P Gain/Passthrough'
 * '<S191>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Ideal P Gain Fdbk/Disabled'
 * '<S192>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Integrator/Discrete'
 * '<S193>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Integrator ICs/External IC'
 * '<S194>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/N Copy/Disabled wSignal Specification'
 * '<S195>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/N Gain/Disabled'
 * '<S196>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/P Copy/Disabled'
 * '<S197>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Parallel P Gain/External Parameters'
 * '<S198>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Reset Signal/Disabled'
 * '<S199>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Saturation/External'
 * '<S200>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Saturation/External/Saturation Dynamic'
 * '<S201>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Saturation Fdbk/Disabled'
 * '<S202>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Sum/Sum_PI'
 * '<S203>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Sum Fdbk/Disabled'
 * '<S204>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tracking Mode/Disabled'
 * '<S205>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tracking Mode Sum/Passthrough'
 * '<S206>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tsamp - Integral/Passthrough'
 * '<S207>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/Tsamp - Ngain/Passthrough'
 * '<S208>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/postSat Signal/Forward_Path'
 * '<S209>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/preInt Signal/Internal PreInt'
 * '<S210>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFoff/PI Controller/preSat Signal/Forward_Path'
 * '<S211>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFon/PI_clamp_fixdtFF'
 * '<S212>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFon/PI_clamp_fixdtFF/Clamping_circuit'
 * '<S213>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFon/PI_clamp_fixdtFF/Integrator'
 * '<S214>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Vd_Calculation/Pi Variant/FFon/PI_clamp_fixdtFF/Saturation_hit'
 * '<S215>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/FOC/FOC_Enabled/Voltage_Mode/Saturation Dynamic1'
 * '<S216>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled'
 * '<S217>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Speed_Mode_Protection'
 * '<S218>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection'
 * '<S219>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Variant Subsystem'
 * '<S220>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection'
 * '<S221>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Speed_Mode_Protection/Saturation Dynamic'
 * '<S222>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller'
 * '<S223>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/Variant Subsystem1'
 * '<S224>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Anti-windup'
 * '<S225>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/D Gain'
 * '<S226>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/External Derivative'
 * '<S227>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Filter'
 * '<S228>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Filter ICs'
 * '<S229>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/I Gain'
 * '<S230>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Ideal P Gain'
 * '<S231>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Ideal P Gain Fdbk'
 * '<S232>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Integrator'
 * '<S233>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Integrator ICs'
 * '<S234>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/N Copy'
 * '<S235>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/N Gain'
 * '<S236>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/P Copy'
 * '<S237>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Parallel P Gain'
 * '<S238>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Reset Signal'
 * '<S239>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Saturation'
 * '<S240>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Saturation Fdbk'
 * '<S241>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Sum'
 * '<S242>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Sum Fdbk'
 * '<S243>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tracking Mode'
 * '<S244>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tracking Mode Sum'
 * '<S245>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tsamp - Integral'
 * '<S246>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tsamp - Ngain'
 * '<S247>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/postSat Signal'
 * '<S248>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/preInt Signal'
 * '<S249>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/preSat Signal'
 * '<S250>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Anti-windup/Back Calculation'
 * '<S251>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/D Gain/Disabled'
 * '<S252>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/External Derivative/Disabled'
 * '<S253>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Filter/Disabled'
 * '<S254>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Filter ICs/Disabled'
 * '<S255>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/I Gain/External Parameters'
 * '<S256>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Ideal P Gain/Passthrough'
 * '<S257>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Ideal P Gain Fdbk/Disabled'
 * '<S258>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Integrator/Discrete'
 * '<S259>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Integrator ICs/External IC'
 * '<S260>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/N Copy/Disabled wSignal Specification'
 * '<S261>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/N Gain/Disabled'
 * '<S262>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/P Copy/Disabled'
 * '<S263>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Parallel P Gain/Disabled'
 * '<S264>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Reset Signal/Disabled'
 * '<S265>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Saturation/External'
 * '<S266>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Saturation/External/Saturation Dynamic'
 * '<S267>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Saturation Fdbk/Disabled'
 * '<S268>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Sum/Passthrough_I'
 * '<S269>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Sum Fdbk/Disabled'
 * '<S270>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tracking Mode/Disabled'
 * '<S271>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tracking Mode Sum/Passthrough'
 * '<S272>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tsamp - Integral/Passthrough'
 * '<S273>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/Tsamp - Ngain/Passthrough'
 * '<S274>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/postSat Signal/Forward_Path'
 * '<S275>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/preInt Signal/Internal PreInt'
 * '<S276>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/PI Controller/preSat Signal/Forward_Path'
 * '<S277>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/Variant Subsystem1/GD32F103'
 * '<S278>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Torque_Mode_Protection/Variant Subsystem1/STM32F103'
 * '<S279>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Variant Subsystem/GD32F103'
 * '<S280>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Variant Subsystem/STM32F103'
 * '<S281>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt'
 * '<S282>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt1'
 * '<S283>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt/Integrator'
 * '<S284>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt/Saturation Dynamic1'
 * '<S285>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt1/Integrator'
 * '<S286>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F05_Field_Oriented_Control/Motor_Limitations/Motor_Limitations_Enabled/Voltage_Mode_Protection/I_backCalc_fixdt1/Saturation Dynamic1'
 * '<S287>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management/COM_Method'
 * '<S288>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management/FOC_Method'
 * '<S289>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management/SIN_Method'
 * '<S290>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management/SIN_Method/Final_Phase_Advance_Calculation'
 * '<S291>' : 'BLDCmotor_FOC_R2017b_fixdt/BLDC_controller/F06_Control_Type_Management/SIN_Method/Final_Phase_Advance_Calculation/Modulo_fixdt'
 */
#endif                                 /* BLDC_controller_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
