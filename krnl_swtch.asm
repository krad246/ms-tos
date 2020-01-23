			.cdecls C
			%{
				#include "definitions.h"
				#include "os.h"
			%}

			.include data_model.h

            .sect ".text:_isr"
            .align 2

			.global run_ptr
			.global hw_stackframe_init

			.global os_sp
			.global os_start
			.global os_tick_init
			.global os_first_task

			.global os_update
			.global os_schedule

			.global os_cleanup

;-------------------------------------------------------------------------------
; void save(void)
; - Saves thread context
;-------------------------------------------------------------------------------
save: .macro
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

		_mov &run_ptr, r15
		_mov sp, 0(r15)
	.endm

;-------------------------------------------------------------------------------
; void restore(void)
; - Restores thread context
;-------------------------------------------------------------------------------
restore: .macro
		_mov &run_ptr, r15		; get top of new context block
		_mov @r15, sp

		.if $DEFINED(__MSP430X__)
			_popm #12, r15	; use popm to accelerate if possible
		.else
			_pop r4
			_pop r5
			_pop r6
			_pop r7
			_pop r8
			_pop r9
			_pop r10
			_pop r11
			_pop r12
			_pop r13
			_pop r14
			_pop r15
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

os_syscall: .macro
			_sub #4, sp				; allocate a trapframe

			; prepare arguments for trapframe init - first back up

			_push r12				; will contain pointer to start of trapframe
			_push r13				; will contain program address of caller
			_push r14				; will contain status register value

			_mov sp, r12			; after backing up original values, load in offset trapframe pointer

			; bring it back to the actual location
			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				_add #12, r12
			.else
				_add #6, r12
			.endif

			_mov 4(r12), r13		; load caller address in as program counter value to program
			_mov sr, r14			; program in the status register

			; create the trapframe
			_call #hw_stackframe_init

			; restore original register values
			_pop r14
			_pop r13
			_pop r12
	.endm

os_preinit: .macro
		os_syscall

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

		_mov sp, &os_sp
	.endm

os_start:
			os_preinit				; back up the pre-os state

			_call #os_first_task	; find the first task and then start it
			restore					; restore context (r4-r15 first, then sp)

			_call #os_tick_init
			
			nop
			reti

os_tick:
			_profile_start			; start timer

			save					; save context (r4 - r15 + sp)
			_call #os_update
			_call #os_schedule		; reschedule
			restore					; restore context (r4-r15 first, then sp)

			_profile_end			; sstop timer

			reti

os_cleanup:
			_mov &os_sp, sp

		.if $DEFINED(__MSP430X__)
			_popm #12, r15	; use popm to accelerate if possible
		.else
			_pop r4
			_pop r5
			_pop r6
			_pop r7
			_pop r8
			_pop r9
			_pop r10
			_pop r11
			_pop r12
			_pop r13
			_pop r14
			_pop r15
		.endif

			nop
			reti

			.sect WDT_VECTOR
			.word os_tick
