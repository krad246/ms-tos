	.cdecls C, "port.c"
	.include port_data_model.h

	.global k_sched_run
	.global k_sched_increment_tick
	.global port_setup_timer_interrupt
	.global k_sched_current_tcb

	.def port_yield
	.def port_start_scheduler

;-----------------------------------------------------------

push_regs: .macro
	.if $DEFINED( __MSP430X__ )
		__pushm #12, r15			; hardware accelerated
	.else
		__push r15					; manual
		__push r14
		__push r13
		__push r12
		__push r11
		__push r10
		__push r9
		__push r8
		__push r7
		__push r6
		__push r5
		__push r4
	.endif
	.endm

pop_regs: .macro
	.if $DEFINED( __MSP430X__ )
		__popm #12, r15
	.else
		__pop r4
		__pop r5
		__pop r6
		__pop r7
		__pop r8
		__pop r9
		__pop r10
		__pop r11
		__pop r12
		__pop r13
		__pop r14
		__pop r15
	.endif
	.endm

;-----------------------------------------------------------

port_save_context: .macro
	push_regs
	__mov &k_sched_current_tcb, r12
	__mov sp, 0(r12)
	.endm

port_restore_context: .macro
	__mov &k_sched_current_tcb, r12
	__mov @r12, sp
	pop_regs

	; any oscillator modifications must be disabled to properly return into the function
	bic.w #(CPUOFF | OSCOFF | SCG0 | SCG1), 0(sp)
	reti
	.endm

;-----------------------------------------------------------

	.sect ".text:_isr"
	.align 2

	.if $DEFINED(PORT_TICK_VECTOR)
		.asg PORT_TICK_VECTOR, port_tick_vector
	.endif

port_tick_isr: .asmfunc
	port_save_context

	__call #k_sched_increment_tick	; increment time
	__call #k_sched_run				; see if any preemption is needed

	port_restore_context
	.endasmfunc

	; assembly vector declaration of preemptive interrupt
	.sect port_tick_vector
	.word port_tick_isr

;-----------------------------------------------------------

	.text
	.align 2

port_manual_context_stack: .macro
	.if $DEFINED ( __LARGE_CODE_MODEL__ )	; 20-bit PC messes up trapframe

		; top 4 bytes need to include RETI-formatted trapframe
		; need to swap top 2 bytes with bottom 2 bytes, then modify the top

		; atomic MOV saves SR at the point of entry
		mov.b sr, 3(sp)

		; once SR backed up, disable interrupts for the rest
		dint
		nop

		; shifts upper 4 bits of PC to higher nibble at sp[2]
		mov.b 2(sp), r12 ; 3
		rpt #4
		rlax.b r12 ; 6
		mov.b r12, 2(sp) ; 3

		; swap PC + SR bytes for RETI format
		swpb 2(sp) ; 4

		; swap PC[0:15] and PC + SR words for RETI format
		mov.w @sp, r12 ; 2
		mov.w 2(sp), 0(sp) ; 5
		mov.w r12, 2(sp) ; 3

	.else
		; 16-bit PC requires simple push for correct trapframe formatting
		__push sr

		dint
		nop
	.endif

	.endm

; manual yield not using the timer
port_yield: .asmfunc

	port_manual_context_stack ; need to simulate an interrupt by stacking context

	port_save_context

	__call #k_sched_run

	port_restore_context

	.endasmfunc

;-----------------------------------------------------------

	.align 2

port_start_scheduler: .asmfunc
	__call #port_setup_timer_interrupt
	port_restore_context
	.endasmfunc

;-----------------------------------------------------------

	.end
