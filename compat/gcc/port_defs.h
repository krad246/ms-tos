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

/**
 * @name Hardware-specific register definitions
 * @{
 */

/**
 * @brief The abstract port register's underlying data type changes based on the code / data model.
 */
//#ifdef __MSP430X_LARGE__
//	typedef uint32_t port_reg_t;
//#else
//	typedef uint16_t port_reg_t;
//#endif

typedef uintptr_t port_reg_t;

/**
 * @brief The abstract port status register's underlying data type is 8 bits for MSP430.
 */
typedef uint8_t port_flags_t;

/**
 * @brief The abstract interrupt hardware stackframe is 4 bytes but the contents depend on code / data model.
 */
typedef union port_iframe {
	uint8_t bytes[4];
	uint16_t words[2];
} port_iframe_t;

/**
 * @brief Underlying data type used for timekeeping.
 */
#if (CONFIG_USE_16_BIT_TICKS == 1)
	typedef uint16_t port_tick_t;
#else
	typedef uint32_t port_tick_t;
#endif

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name CPU context control helpers
 * @{
 */

/**
 * @brief Pushes / saves system registers on the stack. No bookkeeping data maintained.
 */
static inline __attribute__((always_inline)) void port_save_regs(void) {
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
static inline __attribute__((always_inline)) void port_restore_regs(void) {
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
 * @brief Saves system registers and then updates sched_active_thread for calls to port_restore_context().
 */
static inline __attribute__((always_inline)) void port_save_context(void) {

	/* pushes registers r15 -> r4, then sets sched_active_thread */
	#if defined(__MSP430_HAS_MSP430XV2_CPU__)  || defined(__MSP430_HAS_MSP430X_CPU__)
		#ifdef __MSP430X_LARGE__
			__asm__ __volatile__("pushm.a #12, r15");
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

/**
 * @brief Grabs sched_active_thread's bookkeeping data and then pulls system registers off the stack.
 */
static inline __attribute__((always_inline)) void port_restore_context(void) {

	/* grabs sched_active_thread, pops registers r4 -> r15, then returns into the task */
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

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name OS-aware ISR helpers
 * @{
 */

/**
 * @def ISR
 * @brief Defines an OS-aware ISR.
 * @details Must be wrapped by calls to port_enter_isr() and port_exit_isr().
 * @param[in] vector	ISR vector number for the ISR table.
 * @param[in] fn		The name of the function being connected to the ISR.
 */
#define ISR(vector, fn)       __attribute__((naked, interrupt(vector))) void fn(void)

/**
 * @brief ISR entry hook. Switches to a kernel interrupt stack if appropriate, then sets IRQ_IN.
 */
static inline void __attribute__((always_inline)) port_enter_isr(void) {

	/* back up all context on the interrupted task stack */
	port_save_context();

	/* changing to a separate kernel interrupt stack reduces stack overflow potential */
	#ifdef CONFIG_USE_KERNEL_STACK
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("mov.a %0, sp" : : "i"(sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#else
				__asm__ __volatile__("mov.w %0, sp" : : "i"(sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#endif
	#endif

	/* notify that we're in an IRQ */
	sched_set_status_flags(&sched_status_flags, SCHED_FLAG_IN_IRQ);
}

/**
 * @brief ISR exit hook. Clears IRQ_IN, and yields to a higher priority thread if appropriate.
 */
static inline void __attribute__((always_inline)) port_exit_isr(void) {

	/* notify that the IRQ is done */
	sched_clear_status_flags(&sched_status_flags, SCHED_FLAG_IN_IRQ);

	/* if the interrupt awakened a high priority thread, select that for context switch */
    if (sched_status_flags & SCHED_FLAG_CONTEXT_SWITCH_REQUESTED) {
        sched_run();
    }

    /**
     * if a new task was chosen, switch into that one.
     * else switch back into the interrupted stack
     * because sched_active_thread was already updated.
     */
    port_restore_context();
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Interrupt control helpers
 * @{
 */

/**
 * @brief Sets the IRQ disable bit in the status register.
 */
void port_disable_interrupts(void);

/**
 * @brief Clears the IRQ disable bit in the status register.
 */
void port_enable_interrupts(void);

/**
 * @brief Sets the interrupt control flag to the specified value.
 * @param[in] mask	IRQ state to restore to.
 */
void port_set_interrupt_state(port_flags_t mask);

/**
 * @brief Retrieves the value of the IRQ status bit from the status register.
 * @return Value of the status register. Should not be interpreted as a boolean and instead as the raw data.
 * @see port_interrupts_enabled
 */
port_flags_t port_get_interrupt_state(void);

/**
 * @brief Checks if interrupts are currently enabled or disabled. Invokes port_get_interrupt_state().
 * @return True if interrupts are active, false if not.
 */
bool port_interrupts_enabled(void);

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Task and scheduler contract functions
 * @{
 */

/**
 * @brief Configures the stack of a task to look exactly as if a call to port_save_context() was made.
 * @param[in] ptr_stack_top		Pointer to the current top of the task stack.
 * @param[in] ptr_xcode			Pointer to the thread runnable.
 * @param[in] ptr_fn_args		Pointer to the runnable function arguments.
 */
port_reg_t *port_init_stack(port_reg_t *stack_top, thread_fn_t xcode, void *fn_args);

/**
 * @brief Sets up the hardware time slicer.
 * @details Not implemented. Must be provided by user.
 */
void port_setup_timer_interrupt(void);

/**
 * @brief Stops the hardware time slicer.
 * @details Not implemented. Must be provided by user.
 */
void port_disable_timer_interrupt(void);

/**
 * @brief Starts the OS scheduler. Invokes port_setup_timer_interrupt().
 */
void __attribute__((noinline)) port_sched_start(void);

/**
 * @brief Exits the currently running thread and disables the scheduler. Invokes port_disable_timer_interrupt().
 */
void __attribute__((naked)) port_sched_end(void);

/**
 * @brief Manual context switch. Surrenders time slice to the next thread in the run queue.
 */
void port_yield(void);

/**
 * @brief Manual context switch. Surrenders time slice to the highest priority thread in the run queue.
 */
void port_yield_higher(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_GCC_PORT_DEFS_H_ */
