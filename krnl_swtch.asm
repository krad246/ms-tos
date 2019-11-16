			.cdecls "definitions.h"
			.include data_model.h

            .sect ".text:_isr"
            .align 2

			.global run_ptr

			.global _os_switch
			.global _os_start
			.global os_schedule

			.global os_tick_init


;-------------------------------------------------------------------------------
; void _save(void)
; - Saves thread context
;-------------------------------------------------------------------------------
_save: .macro
		_push &_sp_backup

		.if $DEFINED(__MSP430X__)
			_pushm #12, r15	; use pushm to accelerate if possible
		.else
			_push r15
			_push r14
			_push r13
			_push r12
			_push r11
			_push r10
			_push r9
			_push r8
			_push r7
			_push r6
			_push r5
			_push r4
		.endif
	.endm

;-------------------------------------------------------------------------------
; void _restore(void)
; - Restores thread context
;-------------------------------------------------------------------------------
_restore: .macro
		.if $DEFINED(__MSP430X__)
			_popm #12, r15	; use popm to accelerate if possible
		.else
			_pop r15
			_pop r14
			_pop r13
			_pop r12
			_pop r11
			_pop r10
			_pop r9
			_pop r8
			_pop r7
			_pop r6
			_pop r5
			_pop r4
		.endif

		_mov @sp, sp
	.endm

_stack_unwind: .macro	; unwind stack to hit the base
	.if $DEFINED( __LARGE_CODE_MODEL__ ) & $DEFINED(__MSP430X__)
		_add #0x34, sp	; large code model requires larger stackframe
	.else
		_add #0x1a, sp
	.endif
	.endm

;-------------------------------------------------------------------------------
; void _profile_start(void)
; - Starts the context switch time profiler
;-------------------------------------------------------------------------------
_profile_start: .macro
		mov.w #0x004, &TA0CTL
		mov.w #0x220, &TA0CTL
	.endm

;-------------------------------------------------------------------------------
; void _profile_start(void)
; - Halts the context switch time profiler
;-------------------------------------------------------------------------------
_profile_end: .macro
		_mov #0, &TA0CTL
	.endm



_os_start:
			_mov #__STACK_END, sp	; switch to kernel stack to prevent context corruption

			_call #os_tick_init

			_mov &run_ptr, sp		; get top of new context block
			_restore				; restore context (r4-r15 first, then sp)

			nop
			reti

_os_switch:
			;_profile_start			; start timer

			_mov sp, &_sp_backup	; backup sp (will replace with kernel stack)

			_mov &run_ptr, sp		; move sp to context block

			_stack_unwind			; unwind stack for push

			_save					; save context (r4 - r15 + sp)

			_mov #__STACK_END, sp	; switch to kernel stack to prevent context corruption
			_call #os_schedule		; reschedule

			_mov &run_ptr, sp		; get top of new context block

			_restore				; restore context (r4-r15 first, then sp)

			;_profile_end			; start timer

			nop
			reti

			.sect WDT_VECTOR
			.word _os_switch

			.sect .stack
			.global __STACK_END

			.data
_sp_backup:	.word 0
			.end



