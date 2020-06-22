/*
 * internals.h
 *
 *  Created on: Jun 13, 2020
 *      Author: krad2
 */

#ifndef ARCH_INTERNALS_H_
#define ARCH_INTERNALS_H_

#include <msp430.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*-----------------------------------------------------------*/

/**
 * @name CPU context control helpers
 * @{
 */

/**
 * @brief Pushes / saves system registers on the stack. No bookkeeping data maintained.
 */
static inline __attribute__((always_inline)) void arch_save_regs(void) {
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
			__asm__ __volatile__("pushm.a #12, r15");	/* pushes 15 -> 4 */
		#else
			__asm__ __volatile__("pushm.w #12, r15");
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
	#endif
}

/**
 * @brief Pops / pulls system registers off the stack. No bookkeeping data maintained.
 */
static inline __attribute__((always_inline)) void arch_restore_regs(void) {
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("popm.a #12, r15");	/* pops 4 -> 15 */
		#else
				__asm__ __volatile__("popm.w #12, r15");
		#endif
	#else
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
	#endif
}

/**
 * @brief Saves system registers and then updates sched_active_thread for calls to arch_restore_context().
 */
static inline __attribute__((always_inline)) void arch_save_context(void) {

	/* pushes registers r15 -> r4, then sets sched_active_thread */
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
			__asm__ __volatile__("pushm.a #12, r15");
			__asm__ __volatile__("mov.a sp, %0" : "=r"(sched_g.sched_active_thread->sp));
		#else
			__asm__ __volatile__("pushm.w #12, r15");
			__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_g.sched_active_thread->sp));
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

		__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_g.sched_active_thread->sp));
	#endif
}

/**
 * @brief Grabs sched_active_thread's bookkeeping data and then pulls system registers off the stack.
 */
static inline __attribute__((always_inline)) void arch_restore_context(void) {

	/* grabs sched_active_thread, pops registers r4 -> r15, then returns into the task */
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("mov.a %0, sp" : : "m"(sched_g.sched_active_thread->sp));
				__asm__ __volatile__("popm.a #12, r15");	// pops 4 -> 15
		#else
				__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_g.sched_active_thread->sp));
				__asm__ __volatile__("popm.w #12, r15");
		#endif

		__asm__ __volatile__("bic %0, 0(sp)" : : "i"(CPUOFF | OSCOFF | SCG0 | SCG1));
		__asm__ __volatile__("reti");
	#else
		__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_g.sched_active_thread->sp));
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

/** @} */

#endif /* ARCH_INTERNALS_H_ */
