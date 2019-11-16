;---------------------------------------------------------------------------------------------------------------------
; MSP430 Code / Data Model Checks
; Original author: FreeRTOS
; Borrowed from: github.com/bastl-instruments/teensy-RTOS-template/tree/master/FreeRTOS/portable/CCS/MSP430X
;---------------------------------------------------------------------------------------------------------------------
	.if $DEFINED( __LARGE_DATA_MODEL__ ) & $DEFINED(__MSP430X__)	; 20-bit data registers (software stack frame has to be bigger)
		.define "pushm.a", _pushm
		.define "popm.a", _popm
		.define "push.a", _push
		.define "pop.a", _pop
		.define "mov.a", _mov
		.define "add.a", _add
	.else
		.define "pushm.w", _pushm
		.define "popm.w", _popm
		.define "push.w", _push
		.define "pop.w", _pop
		.define "mov.w", _mov
		.define "add.w", _add
	.endif

	.if $DEFINED( __LARGE_CODE_MODEL__ ) & $DEFINED(__MSP430X__)	; 20-bit function pointers, 16-bit data
		.define "calla", _call
		.define "reta", _ret
	.else
		.define "call", _call
		.define "ret", _ret
	.endif
