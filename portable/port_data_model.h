; MSP430 Code / Data Model Checks
; Original author: FreeRTOS
; Borrowed from: github.com/bastl-instruments/teensy-RTOS-template/tree/master/FreeRTOS/portable/CCS/MSP430X

;-----------------------------------------------------------

	.if $DEFINED( __MSP430X__ )

		.if $DEFINED( __LARGE_DATA_MODEL__ )	; 20-bit general registers
			.define "pushm.a", __pushm
			.define "popm.a", __popm
			.define "push.a", __push
			.define "pop.a", __pop
			.define "mov.a", __mov
		.else
			.define "pushm.w", __pushm
			.define "popm.w", __popm
			.define "push.w", __push
			.define "pop.w", __pop
			.define "mov.w", __mov
		.endif

		.if $DEFINED( __LARGE_CODE_MODEL__ )	; 20-bit PC
			.define "calla", __call
			.define "reta", __reti
		.else
			.define "call", __call
			.define "ret", __reti
		.endif

	.else										; No MSP430X instructions - emulate in software

		.define "push.w", __push
		.define "pop.w", __pop
		.define "mov.w", __mov
		.define "call", __call
		.define "ret", __reti

	.endif

;-----------------------------------------------------------
