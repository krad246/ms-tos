;-------------------------------------------------------------------------------
; Author: gta
;    
;  Original code borrowed from krad246
;  https://github.com/krad246/scheduler
;
; MSP430/MSP430X CALLING CONVENTION
;  Callee-saved: SP, R4-R10
;  Caller-saved: R11-R15
;  Function args: R12-R15
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            .text                           ; Assemble into program memory.
            .align 2
            .retain
            .retainrefs

            .global preempt_reset
            .global schedule
            .global run_ptr
            .global num_ctx_switches
            .global context_load
            .global context_save
            .global preempt_firstrun

WDT_ISR:                                    ; Stack: (Top) SR, PC
            nop
            dint                            ; Disable interrupts
            nop
            inc     &num_ctx_switches
            mov.w   &run_ptr, R12           ; get value of running TCB context
            mov.w   R4,   0(R12)            ; Save context
            mov.w   R5,   2(R12)
            mov.w   R6,   4(R12)
            mov.w   R7,   6(R12)
            mov.w   R8,   8(R12)
            mov.w   R9,  10(R12)
            mov.w   R10, 12(R12)
            mov.w   SP,  14(R12)
            mov.w   2(SP), 16(R12)
            ;add.w   #2,  14(R12)            ; Pop PC saved on stack from interrupt
            xor.w   R12, R12                ; clear old run_ptr address (rather hardfault than continue w/ bad addr)
            call    #schedule               ; update run_ptr
            mov.w   &run_ptr, R12           ; get new value of run_ptr
            mov.w   0(R12), R4              ; Load context
            mov.w   2(R12), R5
            mov.w   4(R12), R6
            mov.w   6(R12), R7
            mov.w   8(R12), R8
            mov.w   10(R12), R9
            mov.w   12(R12), R10
            mov.w   14(R12), SP
            call    #preempt_reset          ; reset watchdog timer
            mov.w   16(R12), 2(SP)          ; Put the PC on the stack for RETI
            bis     #GIE, 0(SP)             ; Set GIE bit on SR saved on stack (hack to fix interrupts not working? wtf)
            nop
            eint                            ; Enable interrupts
            nop
            reti                            ; RETI for CPUX (GIE bit set needs pipeline NOP here-- fix)


preempt_firstrun: .asmfunc
            mov.w   &run_ptr, R12           ; get new value of run_ptr
            mov.w   0(R12), R4              ; Load context
            mov.w   2(R12), R5
            mov.w   4(R12), R6
            mov.w   6(R12), R7
            mov.w   8(R12), R8
            mov.w   10(R12), R9
            mov.w   12(R12), R10
            mov.w   14(R12), SP
            ;call    #preempt_reset          ; reset watchdog timer
            nop
            eint                            ; Enable interrupts
            nop
            br      16(R12)
            .endasmfunc


;-------------------------------------------------------------------------------
; void context_save(void *)
;-------------------------------------------------------------------------------
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
            mov.w    @SP, 16(R12)
            add.w    #2,  14(R12)  ; pop pc
            ret
            .endasmfunc

;-------------------------------------------------------------------------------
; void context_load(void *)
;-------------------------------------------------------------------------------
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

            ; WDT ISR
            .sect   ".int49"
            .short  WDT_ISR
            .end


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
