;-------------------------------------------------------------------------------
; Author: gta
;    
;  Original code borrowed from krad246
;  https://github.com/krad246/scheduler
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            .text                           ; Assemble into program memory.
            .align 2

;-------------------------------------------------------------------------------
; void context_save(void *)
;-------------------------------------------------------------------------------
            .global context_save
context_save: .asmfunc
            nop
            dint
            nop
            mov.w    R4,   0(R12)
            mov.w    R5,   2(R12)
            mov.w    R6,   4(R12)
            mov.w    R7,   6(R12)
            mov.w    R8,   8(R12)
            mov.w    R9,  10(R12)
            mov.w    R10, 12(R12)
            mov.w    SP,  14(R12)
            mov.w    PC, 16(R12)
            add.w    #2,  14(R12)  ; pop pc
            ret
            .endasmfunc

;-------------------------------------------------------------------------------
; void context_load(void *)
;-------------------------------------------------------------------------------
            .global context_load
            ;.global WDTCTL_DEFAULT
            ;.global preempt_reset
context_load: .asmfunc
            mov.w   0(R12), R4
            mov.w   2(R12), R5
            mov.w   4(R12), R6
            mov.w   6(R12), R7
            mov.w   8(R12), R8
            mov.w   10(R12), R9
            mov.w   12(R12), R10
            mov.w   14(R12), SP
            ;mov.w   16(R12), R13
            ;mov.w   &WDTCTL_DEFAULT, &WDTCTL
            ;call    #preempt_reset
            nop
            eint
            nop
            br      16(R12)
            .endasmfunc


            ; PORT1_VECTOR            ".int02"                    /* 0xFFE4 Port 1 */
            ; PORT2_VECTOR            ".int03"                    /* 0xFFE6 Port 2 */
            ; ADC10_VECTOR            ".int05"                    /* 0xFFEA ADC10 */
            ; USCIAB0TX_VECTOR        ".int06"                    /* 0xFFEC USCI A0/B0
            ; USCIAB0RX_VECTOR        ".int07"                    /* 0xFFEE USCI A0/B0 Receive */
            ; TIMER0_A1_VECTOR        ".int08"                    /* 0xFFF0 Timer0)A CC1, TA0 */
            ; TIMER0_A0_VECTOR        ".int09"                    /* 0xFFF2 Timer0_A CC0 */
            ; WDT_VECTOR              ".int10"                    /* 0xFFF4 Watchdog Timer */
            ; COMPARATORA_VECTOR      ".int11"                    /* 0xFFF6 Comparator A */
            ; TIMER1_A1_VECTOR        ".int12"                    /* 0xFFF8 Timer1_A CC1-4, TA1 */
            ; TIMER1_A0_VECTOR        ".int13"                    /* 0xFFFA Timer1_A CC0 */
            ; NMI_VECTOR              ".int14"                    /* 0xFFFC Non-maskable */
            ; RESET_VECTOR            ".reset"                    /* 0xFFFE Reset [Highest Priority] */
