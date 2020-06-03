/*
* port_defs.h
*
*  Created on: Jun 2, 2020
*      Author: krad2
*/

#ifndef COMPAT_GCC_PORT_DEFS_H_
#define COMPAT_GCC_PORT_DEFS_H_

#include <msp430.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "port_config.h"

#include "sched.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------*/

// The port register data type changes based on the code / data model
#ifdef __MSP430X_LARGE__
	typedef uint32_t port_reg_t;
#else
	typedef uint16_t port_reg_t;
#endif

// The port status register is an exception - 8 bits are usable, but technically can be 12-bit
typedef uint8_t port_flags_t;

// The port interrupt trapframe whenever an ISR runs - dependent on code and data models
typedef union port_iframe {
	uint8_t bytes[4];
	uint16_t words[2];
} port_iframe_t;

// Time-keeping data type is controlled at build time
#if (CONFIG_USE_16_BIT_TICKS == 1)
	typedef uint16_t port_tick_t;
#else
	typedef uint32_t port_tick_t;
#endif

/*-----------------------------------------------------------*/

static inline __attribute__((always_inline)) void port_save_context() {
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
			__asm__ __volatile__("pushm.a #12, r15");	// pushes 15 -> 4
			__asm__ __volatile__("mov.a sp, %0" : "=r"(sched_active_thread->sp));
		#else
			__asm__ __volatile__("pushm.w #12, r15");
			__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_active_thread->sp));
		#endif
	#else
		__asm__ __volatile__("push.w r15");
		__asm__ __volatile__("push.w r14");
		__asm__ __volatile__("push.w r13");
		__asm__ __volatile__("push.w r12");
		__asm__ __volatile__("push.w r11");
		__asm__ __volatile__("push.w r10");
		__asm__ __volatile__("push.w r9");
		__asm__ __volatile__("push.w r8");
		__asm__ __volatile__("push.w r7");
		__asm__ __volatile__("push.w r6");
		__asm__ __volatile__("push.w r5");
		__asm__ __volatile__("push.w r4");

		__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_active_thread->sp));
	#endif
}

static inline __attribute__((always_inline)) void port_restore_context() {
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("mov.a %0, sp" : : "m"(sched_active_thread->sp));
				__asm__ __volatile__("popm.a #12, r15");	// pops 4 -> 15
		#else
				__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_active_thread->sp));
				__asm__ __volatile__("popm.w #12, r15");
		#endif

		__asm__ __volatile__("bic %0, 0(sp)" : : "i"(CPUOFF | OSCOFF | SCG0 | SCG1));
		__asm__ __volatile__("reti");
	#else
		__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_active_thread->sp));
		__asm__ __volatile__("pop.w r4");
		__asm__ __volatile__("pop.w r5");
		__asm__ __volatile__("pop.w r6");
		__asm__ __volatile__("pop.w r7");
		__asm__ __volatile__("pop.w r8");
		__asm__ __volatile__("pop.w r9");
		__asm__ __volatile__("pop.w r10");
		__asm__ __volatile__("pop.w r11");
		__asm__ __volatile__("pop.w r12");
		__asm__ __volatile__("pop.w r13");
		__asm__ __volatile__("pop.w r14");
		__asm__ __volatile__("pop.w r15");

		__asm__ __volatile__("bic %0, 0(sp)" : : "i"(CPUOFF | OSCOFF | SCG0 | SCG1));
		__asm__ __volatile__("reti");
	#endif
}

/*-----------------------------------------------------------*/

// Defines an OS-aware ISR
#define ISR(a, b)       __attribute__((naked, interrupt(a))) void b(void)

// Interrupt entry / exit hooks for OS-linked ISRs
static inline void __attribute__((always_inline)) port_enter_isr(void) {
    port_save_context();

	#ifdef CONFIG_USE_KERNEL_STACK
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("mov.a %0, sp" : : "i"(sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#else
				__asm__ __volatile__("mov.w %0, sp" : : "i"(sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#endif
	#endif

	sched_set_status_flags(&sched_status_flags, SCHED_FLAG_IN_IRQ);
}

static inline void __attribute__((always_inline)) port_exit_isr(void) {
	sched_clear_status_flags(&sched_status_flags, SCHED_FLAG_IN_IRQ);
	// may have added thread to run queue that was blocking and waiting for another signal / sleeping indefinitely

	//thread_yield and thread_yield higher work on the preemptive side i.e. yield advances through the scheduler steps faster while thread_yield_higher resets to the top of the run queue
    if (sched_status_flags & SCHED_FLAG_CONTEXT_SWITCH_REQUESTED) { // needs to yield to highest thread that the interrupt may have scheduled
        sched_run();
    	// sched_yield_higher();
    }

    port_restore_context();
}

/*-----------------------------------------------------------*/

// Interrupt control macros

#define PORT_FLAG_INTERRUPTS_ENABLED ((port_flags_t) GIE)

void port_enable_interrupts(void);

void port_disable_interrupts(void);

port_flags_t port_get_interrupt_state(void);

void port_set_interrupt_state(port_flags_t mask);

bool port_interrupts_enabled(void);

/*-----------------------------------------------------------*/

// Task and scheduler utilities

// delete / refactor to port_
void app_setup_timer_interrupt(void);

// tickless will require 2nd timer

// move to thread.h
port_reg_t *port_init_stack(port_reg_t *ptr_stack_top, thread_fn_t ptr_xcode, void *ptr_fn_args);

void port_sched_start(void);

void port_sched_end(void);

void port_yield(void);

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_GCC_PORT_DEFS_H_ */
