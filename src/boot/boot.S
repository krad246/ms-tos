;-------------------------------------------------------------------------------
; MS-TOS MSP430 boot code
;-------------------------------------------------------------------------------
            
		#include "msp430.h"
		#include "memmodel.h"

;-------------------------------------------------------------------------------
            
		.text                           
		.global _start					
		.global main

;-------------------------------------------------------------------------------

_start:      
		mov.w  	#__stack, sp					; initialize stack pointer

		;-------------------

		dint									; enter critical section
		nop

		;-------------------

		mov.w	#(WDTPW | WDTHOLD), &WDTCTL 	; freeze watchdog
		
		;-------------------

		mov.w	#__bssstart, r12			 
		clr.w	r13
		mov.w	#__bsssize, r14

	.ifdef __MSP430X_LARGE__
		clr.w	R15									
	.endif

		call	#memset							; zero-init .bss

		;-------------------

		mov.w	#__datastart, r12
		mov.w	#__romdatastart, r14

		cmp.w	r12, r13						; skip if no memory
												; needs to be moved
		jeq		_call_main
		mov.w	#__romdatacopysize, r14
	
	.ifdef __MSP430X_LARGE__
		clr.w	R15									
	.endif

		call	#memmove						; copy rom to ram

		;-------------------

_call_main:; Set argc == 0
		clr.w	R12									
		call	#main

_main_exit:
		bis.w	#LPM3_bits,	sr
		nop

		jmp		_main_exit

;-------------------------------------------------------------------------------
;           stack pointer definition
;-------------------------------------------------------------------------------
            
		; .global __stack
        .sect 	.stack

;-------------------------------------------------------------------------------
;           interrupt vectors
;-------------------------------------------------------------------------------
        .sect   ".resetvec"            
        .word  	_start

		.end
