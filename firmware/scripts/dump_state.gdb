# GDB script — attach to a live/frozen board and dump post-mortem state.
# Loaded by attach_and_dump.bat; do not run manually.
# Pure GDB commands only (this GDB build has no Python).

target extended-remote localhost:3333
monitor halt

set pagination off
set print pretty on
set confirm off

printf "\n"
printf "================================================================\n"
printf " HOVERBOARD FIRMWARE POST-MORTEM DUMP\n"
printf "================================================================\n"

printf "\n-------- PROGRAM COUNTER / FUNCTION --------\n"
# Print the essentials with expressions (immune to differing register-name schemes).
printf "PC     = 0x%08x\n", $pc
printf "LR     = 0x%08x\n", $lr
printf "SP     = 0x%08x\n", $sp
# Read xPSR via OpenOCD (bypasses GDB register-name limitations).
monitor reg xPSR

printf "\n-------- ALL CPU REGISTERS --------\n"
info registers

printf "\n-------- BACKTRACE (top of stack) --------\n"
bt 20

printf "\n-------- SOURCE AT PC --------\n"
list *$pc

# ARM SCB fault registers — non-zero means a fault handler was entered
printf "\n-------- ARM SCB FAULT REGISTERS --------\n"
printf "CFSR   = 0x%08x  (Configurable Fault Status)\n", *(unsigned int*)0xE000ED28
printf "HFSR   = 0x%08x  (HardFault Status)\n",          *(unsigned int*)0xE000ED2C
printf "DFSR   = 0x%08x  (Debug Fault Status)\n",        *(unsigned int*)0xE000ED30
printf "MMFAR  = 0x%08x  (MemManage Fault Address)\n",   *(unsigned int*)0xE000ED34
printf "BFAR   = 0x%08x  (Bus Fault Address)\n",         *(unsigned int*)0xE000ED38
printf "SHCSR  = 0x%08x  (System Handler Control)\n",    *(unsigned int*)0xE000ED24

# Our own fault_info capture (populated by our fault handlers)
printf "\n-------- fault_info[] (captured by our handler) --------\n"
printf " [0]=type (0=none 1=HF 2=BF 3=UF 4=MM)  [1]=CFSR  [2]=HFSR\n"
printf " [3]=MMFAR  [4]=BFAR  [5]=SHCSR  [6..7]=reserved\n"
output/x fault_info
printf "\n"

# SysTick — is the tick even running?
printf "\n-------- SysTick --------\n"
printf "CTRL   = 0x%08x  (bit0=ENABLE bit1=TICKINT bit16=COUNTFLAG)\n", *(unsigned int*)0xE000E010
printf "LOAD   = 0x%08x\n", *(unsigned int*)0xE000E014
printf "VAL    = 0x%08x\n", *(unsigned int*)0xE000E018
printf "uwTick = %u\n", uwTick

# ICSR — shows currently active exception and any pending system exceptions
printf "\n-------- SCB->ICSR --------\n"
printf "ICSR   = 0x%08x  (bits0-8=VECTACTIVE bits12-20=VECTPENDING bit26=PENDSTSET bit25=PENDSTCLR)\n", *(unsigned int*)0xE000ED04
# SHPR3 — priority of SysTick and PendSV
printf "SHPR3  = 0x%08x  (bits24-31=SysTick priority, 16-23=PendSV)\n", *(unsigned int*)0xE000ED20

# NVIC pending/active — which IRQs are queued or executing
printf "\n-------- NVIC IRQ priorities (byte per IRQ) --------\n"
printf "IRQ 11 (DMA1_Ch1 FOC) prio = 0x%02x\n", *(unsigned char*)(0xE000E400 + 11)
printf "IRQ 18 (ADC1_2 wdg)   prio = 0x%02x\n", *(unsigned char*)(0xE000E400 + 18)
printf "IRQ 31 (I2C1_EV)      prio = 0x%02x\n", *(unsigned char*)(0xE000E400 + 31)
printf "IRQ 32 (I2C1_ER)      prio = 0x%02x\n", *(unsigned char*)(0xE000E400 + 32)
printf "IRQ 55 (TIM7)         prio = 0x%02x\n", *(unsigned char*)(0xE000E400 + 55)
printf "  (upper nibble = actual priority; 0x00=highest 0xF0=lowest)\n"

printf "\n-------- NVIC pending IRQs (ISPR0..2) --------\n"
printf "ISPR0  = 0x%08x  (IRQ 0..31)\n",  *(unsigned int*)0xE000E200
printf "ISPR1  = 0x%08x  (IRQ 32..63)\n", *(unsigned int*)0xE000E204
printf "ISPR2  = 0x%08x  (IRQ 64..95)\n", *(unsigned int*)0xE000E208

printf "\n-------- NVIC active IRQs (IABR0..2) --------\n"
printf "IABR0  = 0x%08x  (IRQ 0..31 — non-zero means IRQ was running)\n",  *(unsigned int*)0xE000E300
printf "IABR1  = 0x%08x  (IRQ 32..63)\n", *(unsigned int*)0xE000E304
printf "IABR2  = 0x%08x  (IRQ 64..95)\n", *(unsigned int*)0xE000E308

# I2C peripherals snapshot
printf "\n-------- I2C1 (Y sensor bus) --------\n"
printf "CR1    = 0x%04x   CR2    = 0x%04x\n", *(unsigned short*)0x40005400, *(unsigned short*)0x40005404
printf "SR1    = 0x%04x   SR2    = 0x%04x\n", *(unsigned short*)0x40005414, *(unsigned short*)0x40005418
printf "CCR    = 0x%04x   TRISE  = 0x%04x\n", *(unsigned short*)0x4000541C, *(unsigned short*)0x40005420

printf "\n-------- I2C2 (X sensor bus) --------\n"
printf "CR1    = 0x%04x   CR2    = 0x%04x\n", *(unsigned short*)0x40005800, *(unsigned short*)0x40005804
printf "SR1    = 0x%04x   SR2    = 0x%04x\n", *(unsigned short*)0x40005814, *(unsigned short*)0x40005818

# USART3 — TX/RX status
printf "\n-------- USART3 (debug/comm) --------\n"
printf "SR     = 0x%04x  (bit6=TC bit7=TXE bit5=RXNE)\n", *(unsigned short*)0x40004800
printf "CR1    = 0x%04x\n", *(unsigned short*)0x4000480C
printf "CR3    = 0x%04x\n", *(unsigned short*)0x40004814

# TIM1/TIM8 BDTR MOE bit — motor PWM state
printf "\n-------- Motor PWM enable (BDTR.MOE) --------\n"
printf "TIM1 BDTR (right motor) = 0x%04x  (bit15 MOE=motor outputs enabled)\n", *(unsigned short*)0x40012C44
printf "TIM8 BDTR (left  motor) = 0x%04x\n", *(unsigned short*)0x40013444

# Firmware-level global state
printf "\n-------- FIRMWARE STATE --------\n"
printf "buzzerTimer            = %u\n",  (unsigned)buzzerTimer
printf "manual_cmd_active      = %d\n",  (int)manual_cmd_active
printf "manual_cmd_val         = %d\n",  (int)manual_cmd_val
printf "pwml                   = %d\n",  (int)pwml
printf "pwmr                   = %d\n",  (int)pwmr
printf "enable                 = %d\n",  (int)enable
printf "mt6701_y_recovery_req  = %d\n",  (int)mt6701_y_recovery_req

# Bare-metal I2C1 driver counters — critical for post-mortem freeze analysis.
# If one bm_state_visits[N] dominates by orders of magnitude vs cplt_count,
# state N is where the IRQ loop got trapped.
printf "\n-------- MT6701 Y bare-metal driver counters --------\n"
printf "mt6701_y_cplt_count            = %u  (successful reads)\n", (unsigned)mt6701_y_cplt_count
printf "mt6701_y_err_count             = %u  (ER IRQ + spurious EV IRQ)\n", (unsigned)mt6701_y_err_count
printf "mt6701_y_bm_state_max          = %u  (highest state ever seen)\n", (unsigned)mt6701_y_bm_state_max
printf "mt6701_y_bm_watchdog_hits      = %u  (state stuck > 20 ms)\n", (unsigned)mt6701_y_bm_watchdog_hits
printf "i2c1_ev_irq_hits               = %u  (I2C1_EV_IRQ handler entries)\n", (unsigned)i2c1_ev_irq_hits
printf "i2c1_er_irq_hits               = %u  (I2C1_ER_IRQ handler entries)\n", (unsigned)i2c1_er_irq_hits
printf "bm_state_visits[0=IDLE]        = %u\n", (unsigned)mt6701_y_bm_state_visits[0]
printf "bm_state_visits[1=SB_W]        = %u\n", (unsigned)mt6701_y_bm_state_visits[1]
printf "bm_state_visits[2=ADDR_W]      = %u\n", (unsigned)mt6701_y_bm_state_visits[2]
printf "bm_state_visits[3=BTF_MEM]     = %u\n", (unsigned)mt6701_y_bm_state_visits[3]
printf "bm_state_visits[4=SB_R]        = %u\n", (unsigned)mt6701_y_bm_state_visits[4]
printf "bm_state_visits[5=ADDR_R]      = %u\n", (unsigned)mt6701_y_bm_state_visits[5]
printf "bm_state_visits[6=BTF_DATA]    = %u\n", (unsigned)mt6701_y_bm_state_visits[6]
printf "bm_state_visits[7=default]     = %u\n", (unsigned)mt6701_y_bm_state_visits[7]

printf "\n-------- MT6701 X bare-metal driver counters --------\n"
printf "mt6701_x_cplt_count            = %u\n", (unsigned)mt6701_x_cplt_count
printf "mt6701_x_err_count             = %u\n", (unsigned)mt6701_x_err_count
printf "mt6701_x_bm_state_max          = %u\n", (unsigned)mt6701_x_bm_state_max
printf "mt6701_x_bm_watchdog_hits      = %u\n", (unsigned)mt6701_x_bm_watchdog_hits
printf "i2c2_ev_irq_hits               = %u\n", (unsigned)i2c2_ev_irq_hits
printf "i2c2_er_irq_hits               = %u\n", (unsigned)i2c2_er_irq_hits
printf "bm_state_visits[0=IDLE]        = %u\n", (unsigned)mt6701_x_bm_state_visits[0]
printf "bm_state_visits[1=SB_W]        = %u\n", (unsigned)mt6701_x_bm_state_visits[1]
printf "bm_state_visits[2=ADDR_W]      = %u\n", (unsigned)mt6701_x_bm_state_visits[2]
printf "bm_state_visits[3=BTF_MEM]     = %u\n", (unsigned)mt6701_x_bm_state_visits[3]
printf "bm_state_visits[4=SB_R]        = %u\n", (unsigned)mt6701_x_bm_state_visits[4]
printf "bm_state_visits[5=ADDR_R]      = %u\n", (unsigned)mt6701_x_bm_state_visits[5]
printf "bm_state_visits[6=BTF_DATA]    = %u\n", (unsigned)mt6701_x_bm_state_visits[6]
printf "bm_state_visits[7=default]     = %u\n", (unsigned)mt6701_x_bm_state_visits[7]

printf "\n-------- encoder_y (full struct) --------\n"
output encoder_y
printf "\n"

printf "\n================================================================\n"
printf " END DUMP — CPU is halted. Session will disconnect now.\n"
printf "================================================================\n"

quit
