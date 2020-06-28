/*
 * hal.c
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#include "hal.h"

/*-----------------------------------------------------------*/

/**
 * @name MSP430 hardware-specific OS feature configuration.
 * @{
 */

/* MSP430 hardware timer-specific interrupt vectors */
#define ARCH_TICK_VECTOR 								TIMER0_A0_VECTOR
#define ARCH_TIMEKEEPING_VECTOR							TIMER0_A1_VECTOR

/* Attributes of our timer setup */
#define ARCH_TICK_CLK_FREQ								4096
#define ARCH_TIME_MAX_SECONDS							(unsigned int) (65536.0 / ARCH_TICK_CLK_FREQ)

/* Equivalent period for the configured tick rate */
#define CONFIG_TICK_RATE_MS								(unsigned int) (1000.0 / CONFIG_TICK_RATE_HZ)

/* Calculate hardware parameters to achieve the constants above, with or w / o approximation */
#define ROUND(x) 										((x) >= 0 ? (long) ((x) + 0.5) : (long) ((x) - 0.5))

#if (CONFIG_USE_FAST_MATH == 1)
	#define __ARCH_FAST_MATH_BINARY_POINT				(10)
	#define __ARCH_FAST_MATH_1000						((unsigned long) (1ul << (__ARCH_FAST_MATH_BINARY_POINT)))
	#define __ARCH_FAST_MATH_500						((unsigned long) (1ul << ((__ARCH_FAST_MATH_BINARY_POINT) - 1)))
	#define __ARCH_FAST_MATH_8							(ROUND((__ARCH_FAST_MATH_1000) / 125.0))
	#define __ARCH_FAST_ROUND(val, bpoint)				(((((unsigned long) (val)) + (__ARCH_FAST_MATH_500))) >> (bpoint))
	#define __ARCH_MS_TO_CYCLES_1_8(ms)					((((unsigned long) ARCH_TICK_CLK_FREQ) * ((unsigned long) (ms))) >> 3)
	#define __ARCH_MS_TO_CYCLES_1_125(ms)				(((unsigned long) (ms)) * (((unsigned long) __ARCH_FAST_MATH_8)))
	#define __ARCH_MS_TO_CYCLES_1_1000(ms)				__ARCH_MS_TO_CYCLES_1_125( __ARCH_MS_TO_CYCLES_1_8((ms)))
	#define __ARCH_MS_TO_CYCLES(ms)						__ARCH_FAST_ROUND(__ARCH_MS_TO_CYCLES_1_1000((ms)), __ARCH_FAST_MATH_BINARY_POINT)
	#define ARCH_MS_TO_CYCLES(ms)						__ARCH_MS_TO_CYCLES((ms))
#else
	#define ARCH_MS_TO_CYCLES(ms)						ROUND(((double) ms) * (ARCH_TICK_CLK_FREQ / 1000.0))
#endif

/*-----------------------------------------------------------*/

/**
 * @brief Sets up OS time slicing interrupt.
 * @param[in] ms period of timer interrupt, measured in cycles.
 */
static inline void arch_setup_os_timer(unsigned int period) {
	TA0CTL = MC_0 | TACLR;
	TA0CCR0 = period;
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_1 | ID_3 | MC_2;
}

/**
 * @brief Stops and clears the OS timer.
 */
static inline void arch_disable_os_timer(void) {
	TA0CTL = MC_0 | TACLR;
}

inline unsigned int arch_time_now(void) {
	return TA0R;
}

/*---*/

/**
 * @brief Acknowledges a wakeup interrupt, but doesn't stop the system timer.
 */
static inline void arch_acknowledge_wakeup_interrupt(void) {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR1 &= ~CCIFG;
	#endif
}

/**
 * @brief Sets up an interrupt at the specified time.
 * @param[in] next_wake_time When to expect a thread awakening, measured in cycles.
 */
static inline void arch_schedule_next_wakeup(unsigned int next_wake_time) {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR1 = next_wake_time;
		TA0CCTL1 |= CCIE;
	#else
		(void) next_wake_time;
	#endif
}

/**
 * @brief Masks tick interrupts, but doesn't stop the tick timer.
 */
static inline void arch_suppress_wakeup_interrupt(void) {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR1 &= ~CCIE;
	#endif
}

/*---*/

/**
 * @brief Acknowledges a tick interrupt, but doesn't stop the tick timer.
 */
static inline void arch_acknowledge_tick_interrupt(void) {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR0 &= ~CCIFG;
	#endif
}

/**
 * @brief Sets up an interrupt so the next context switch will occur.
 * @param[in] next_wake_time When to expect a thread awakening, measured in cycles.
 */
static inline void arch_schedule_next_tick(unsigned int next_tick_time) {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR0 = TA0R + next_tick_time;
		TA0CCTL0 |= CCIE;
	#endif
}

/**
 * @brief Masks wakeup interrupt, but doesn't stop the time keeper.
 */
static inline void arch_suppress_tick_interrupt() {
	#if (CONFIG_USE_TICKLESS_IDLE == 1)
		TA0CCR0 &= ~CCIE;
	#endif
}

/*---*/

/**
 * @brief Sets up the watchdog for the watchdog monitor.
 */
static inline void arch_setup_watchdog_interrupt(void) {
	#if (CONFIG_WATCHDOG_MONITOR == 1)
		WDTCTL = WDT_ARST_1000;
		SFRIE1 |= WDTIE;
	#endif
}

/*---*/

/**
 * @brief Puts the machine into a low power condition.
 */
void arch_idle(void) {
	__low_power_mode_3();
}

/** @} */


/*-----------------------------------------------------------*/

/**
 * @name MSP430 ISA-specific stack initialization.
 * @{
 */

/**
 * @brief Arch-specific task exit handler. Captures important state parameters, then transfers control to the scheduler.
 */
static void __attribute__((naked, noreturn)) arch_thread_exit(int exit_code) {
	arch_disable_interrupts();
	sched_impl_thread_exit();
	arch_restore_context();

	while (1) {
		panic(PANIC_EXPECT_FAIL, "arch_task_exit() failed");
	}
}

/**
 * @brief Creates a stackframe for fast returns into tasks, emulating an interrupt.
 */
static void arch_create_irq_frame(arch_iframe_t *frame,
		thread_fn_t xcode, arch_flags_t cpu_flags) {

	/* 20-bit MSP430s concatenate the SR with the 20-bit PC in a 32-bit trapframe */
	#ifdef __MSP430X_LARGE__

		/* Split 20-bit function address into 16-bit and 4-bit segments for appropriate placement */
		const arch_reg_t addr_runnable = (arch_reg_t) xcode;
		const uint16_t addr_runnable_l = addr_runnable & 0x0000FFFF;
		const uint16_t addr_runnable_h = (addr_runnable & 0x000F0000) >> 16;

		/**
		 * The highest word address contains the lower 16-bits of the PC in LE format.
		 * The lowest word address contains the SR at the lowest byte.
		 * The other byte has the PC top bits at the high nibble, with other nibble being reserved.
		 */

		/* Highest word address */
		frame->words[1] = addr_runnable_l;

		/* Lowest word & byte address */
		frame->bytes[0] = (uint8_t) cpu_flags;

		/* The remaining byte has the PC at the high nibble */
		frame->bytes[1] = (uint8_t) (addr_runnable_h << 4);
	#else
		/* 16-bit MSP430s store the SR at the next word below the function address in the same size */
		frame->words[1] = (uint16_t) xcode;
		frame->words[0] = (uint8_t) cpu_flags;
	#endif
}

/**
 * @brief Sets up a task stack for thread execution.
 */
volatile arch_reg_t *arch_init_stack(volatile arch_reg_t *stack_top,
		volatile thread_fn_t xcode, volatile void *fn_args) {

	/* Precondition: stack_top points to the bottom of the task stack. */

	/**
	 * Preallocate a constant number of bytes for the interrupt frame,
	 * then create the frame centered on that memory space at stack_top.
	 */
	const uint8_t *stack_base = (uint8_t *) stack_top;

	/* allocate a word for the exit handler */
	stack_top--;
	*stack_top = (arch_reg_t) arch_thread_exit;

	/* allocate an interrupt frame for context switching */
	stack_top = (arch_reg_t *) (stack_base - sizeof(arch_iframe_t));
	arch_create_irq_frame((arch_iframe_t *) stack_top, xcode, GIE);

	/**
	 * Next, preallocate 1 register at a time and preload it with a value.
	 * R12 is an exception because the ABI specifies it as the 1st argument register.
	 */

	stack_top--;							/* Preallocate R15 */
	*stack_top = (arch_reg_t) 0xeeeee; 		/* Preload R15 to 0xEEEEE */

	stack_top--;
	*stack_top = (arch_reg_t) 0xddddd;

	stack_top--;
	*stack_top = (arch_reg_t) 0xccccc;

	stack_top--;							/* Preallocate R12 */
	*stack_top = (arch_reg_t) fn_args;		/* Preload R12 to fn_args */

	stack_top--;
	*stack_top = (arch_reg_t) 0xbbbbb;

	stack_top--;
	*stack_top = (arch_reg_t) 0xaaaaa;

	stack_top--;
	*stack_top = (arch_reg_t) 0x99999;

	stack_top--;
	*stack_top = (arch_reg_t) 0x88888;

	stack_top--;
	*stack_top = (arch_reg_t) 0x77777;

	stack_top--;
	*stack_top = (arch_reg_t) 0x66666;

	stack_top--;
	*stack_top = (arch_reg_t) 0x55555;

	/* Last register is R4 at the top of the stack. */
	stack_top--;
	*stack_top = (arch_reg_t) 0x44444;

	return stack_top;
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name MSP430 ISA-specific scheduler initialization (and preparations for arch_sched_end()).
 * @{
 */

/**
 * @brief Boots up the scheduler and saves pre-boot state for quitting.
 */
void __attribute__((noinline)) arch_sched_start(void) {
	extern volatile sched_impl_t sched_p;

	/* Entering a critical section. */
	arch_disable_interrupts();

	/* Back up the context before booting up the kernel so that we can return later. */
	arch_save_regs();
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a sp, %0" : "=r"(sched_p.boot_context));
	#else
		__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_p.boot_context));
	#endif

	/* Set up the time slicing and safety hardware. */
	arch_setup_os_timer(ARCH_MS_TO_CYCLES(CONFIG_TICK_RATE_MS));
	#if (CONFIG_WATCHDOG_MONITOR == 1)
		arch_setup_watchdog_monitor();
	#endif

	/* Find and return into the first task. */
	sched_impl_run();
	arch_restore_context();
}

/**
 * @brief Exits all threads to the pre-booting location.
 */
void __attribute__((naked)) arch_sched_end(void) {
	extern volatile sched_impl_t sched_p;

	/* Entering a critical section. */
	arch_disable_interrupts();
	arch_disable_os_timer();

	/* Reload the pre-boot state so we can return back to the OS spawn point. */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a %0, sp" : : "m"(sched_p.boot_context));
	#else
		__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_p.boot_context));
	#endif
	arch_restore_regs();

	/**
	 * After restoring registers, the noinline'd arch_sched_start() should have definitely
	 * placed a return address on the stack that we can directly return into.
	 */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("reta");
	#else
		__asm__ __volatile__("ret");
	#endif
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @brief Kernel panic function.
 * TODO: __VA_ARGS__ semantics
 */
static char panic_log[CONFIG_PANIC_DUMP_SIZE];
void __attribute__((noreturn, noinline)) arch_panic(panic_code_t crash_code, const char *message) {
	volatile arch_reg_t caller = 0;

	/**
	 * The noinline specifier forces panic() to ALWAYS push 2 words on the stack.
	 * Additionally, the size of a generic call instruction is 4 bytes, so the very first panic() call
	 * will cause the return PC to point 4 bytes ahead. Thus, we subtract 4 for the faulting address.
	 */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("movx.a 8(sp), %0 " : : "m"(caller));
	#else
		__asm__ __volatile__("mov.w 4(sp), %0" : : "m"(caller));
	#endif
	caller -= 4;

	/* Put the crash reason in the panic log so we can see the cause as well as the faulting address. */
	snprintf(panic_log, CONFIG_PANIC_DUMP_SIZE, "%s (%p)", message, (void *) caller);

	/* Handle the crash reasons differently. */
	switch (crash_code) {
		case PANIC_GENERAL_ERROR: break;
		case PANIC_SOFT_REBOOT: break;
		case PANIC_HARD_REBOOT: break;
		case PANIC_ASSERT_FAIL: break;		/* get what failed */
		case PANIC_EXPECT_FAIL: break;		/* get what failed */
		case PANIC_SSP: break;				/* get the thread that faulted and its OOB condition */
		case PANIC_UNDEFINED: break;
	}

	/* Depending on configuration, either halt or reset. */
	#if (CONFIG_DEBUG_MODE == 1)
		while (1) __bis_SR_register(LPM4_bits & ~GIE);
	#else
		arch_system_reset();
	#endif
}

/*-----------------------------------------------------------*/

/**
 * @name MSP430 task switching intrinsics.
 * @{
 */

/**
 * Yield needs to emulate the stacking behavior of a preemptive tick.
 * Since a tick pushes the PC + SR on the stack, we also need to do that.
 * As this it is a function call, we have to manually push the SR.
 * The rest reads like a standard context switch.
 * Therefore, the following helper functions are defined to take care of this.
 */
static inline void __attribute__((always_inline)) arch_context_switch_prologue(void) {

	/**
	 * Save the SR. The 20-bit trapframe stores its SR in a weird chunk of the frame.
	 */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.b sr, 3(sp)");
	#else
		__asm__ __volatile__("push sr");
	#endif

	/* PC + SR is stacked so interrupts are disabled */
	__disable_interrupt();
}

/**
 * @brief Performs a manual context switch emulating a hardware context switch.
 */
static inline void __attribute__((always_inline)) arch_build_trapframe(void) {

	/**
	 * The 20-bit trapframe is bizarre as previously outlined,
	 * so additional work needs to be done to format a normal function address into
	 * something RETI can work with.
	 *
	 * The 16-bit trapframe doesn't need any work at all.
	 */
	#ifdef __MSP430X_LARGE__

		/* R12 is used as a temp variable so it needs to be backed up. */
		__asm__ __volatile__("pushx.a r12");

		/* Shifts upper 4 bits of PC to higher nibble at SP + 2. */
		__asm__ __volatile__("mov.b 6(sp), r12");
		__asm__ __volatile__("rpt #4");
		__asm__ __volatile__("rlax.b r12");
		__asm__ __volatile__("mov.b r12, 6(sp)");

		/* Swap PC and SR bytes so that the SR moves to a lower address */
		__asm__ __volatile__("swpb 6(sp)");

		/**
		 * Swap adjacent words so that (PC + SR) moves to the lowest address
		 * and the low bits of the PC are at higher addresses.
		 */
		__asm__ __volatile__("mov.w 6(sp), r12");
		__asm__ __volatile__("mov.w 4(sp), 6(sp)");
		__asm__ __volatile__("mov.w r12, 4(sp)");

		/* Restore R12, we don't need it anymore. */
		__asm__ __volatile__("popx.a r12");

	#endif
}

/**
 * @brief Changes to any other runnable thread.
 */
void __attribute__((noinline, naked)) arch_yield(void) {
	extern volatile sched_impl_t sched_p;

	/* Manual context stacking. */
	arch_context_switch_prologue();
	arch_build_trapframe();

	/* Standard context switching logic. */
	arch_save_context();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find a new thread of any priority. */
	sched_impl_yield();
	#if (CONFIG_DEBUG_MODE == 1)
		if (sched_p.sched_active_thread == NULL) panic(PANIC_EXPECT_FAIL, "NULL thread was scheduled");
	#endif

	/* Check if the newly scheduled thread holds a critical section, and restore the lock if true. */
	if (sched_p.state & SCHED_STATUS_IRQ_LOCKED) {

		/* We need to modify the stack, so we need to get the location of the interrupt status register. */
		const register volatile uint8_t *stack_top = (uint8_t *) sched_p.sched_active_thread->sp;
		const register unsigned int iframe_offs = offsetof(arch_context_t, task_addr);
		register volatile uint8_t *arch_iframe_pos = (uint8_t *) (stack_top + iframe_offs);

		/**
		 * Disabling interrupts will allow the new thread to proceed uninterrupted
		 * since it must have yielded in a critical section.
		 */
		*arch_iframe_pos &= ~ARCH_FLAG_INTERRUPTS_ENABLED;
	}

	/* Return into the thread. */
	arch_restore_context();
}

/**
 * @brief Changes to a higher priority thread.
 */
void __attribute__((noinline, naked)) arch_yield_higher(void) {
	extern volatile sched_impl_t sched_p;

	/* Manual context stacking. */
	arch_context_switch_prologue();
	arch_build_trapframe();

	/* Standard context switching logic. */
	arch_save_context();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find a higher priority thread and return into it if possible. */
	sched_impl_yield_higher();
	#if (CONFIG_DEBUG_MODE == 1)
		if (sched_p.sched_active_thread == NULL) panic(PANIC_EXPECT_FAIL, "NULL thread was scheduled");
	#endif

	/* Check if the newly scheduled thread holds a critical section, and restore the lock if true. */
	if (sched_p.state & SCHED_STATUS_IRQ_LOCKED) {

		/* We need to modify the stack, so we need to get the location of the interrupt status register. */
		const register volatile uint8_t *stack_top = (uint8_t *) sched_p.sched_active_thread->sp;
		const register unsigned int iframe_offs = offsetof(arch_context_t, task_addr);
		register volatile uint8_t *arch_iframe_pos = (uint8_t *) (stack_top + iframe_offs);

		/**
		 * Disabling interrupts will allow the new thread to proceed uninterrupted
		 * since it must have yielded in a critical section.
		 */
		*arch_iframe_pos &= ~ARCH_FLAG_INTERRUPTS_ENABLED;
	}

	/* Return into the thread. */
	arch_restore_context();
}

/**
 * @brief Puts the current thread to sleep by scheduling a wakeup at wake_time.
 * @param[in] wake_time the time, in cycles, that the thread will be put back on the run queue.
 */
static void arch_sleep_until(unsigned int wake_time) {
	extern volatile sched_impl_t sched_p;

	/* Pull the next deadline off the sleeping queue (which is in sorted order, then set the wakeup timer for then. */
	thread_impl_t *next_waker = sleep_queue_peek((sleep_queue_t *) &sched_p.sleep_mgr);
	#if (CONFIG_DEBUG_MODE == 1)
		if (next_waker == NULL) {
			panic(PANIC_ASSERT_FAIL, "arch_sleep_until() could not schedule a new thread wakeup because it was NULL");
		}
	#endif
	arch_schedule_next_wakeup(next_waker->sq_entry.wake_time);

	/* Transfer control to a new thread. */
	arch_yield();
}

/**
 * @brief Puts the thread to sleep for the given period of time.
 * @param[in] ms How long to sleep for, in milliseconds.
 */
void arch_sleep_for(unsigned int ms) {
	unsigned int now = arch_time_now();
	unsigned int wake_time = now + ARCH_MS_TO_CYCLES(ms);
	sched_impl_sleep_until(wake_time);
	arch_sleep_until(wake_time);
}


/**
 * @brief Scheduler preemption tick. Invokes sched_run() to distribute time slices.
 */
__attribute__((naked, interrupt(ARCH_TICK_VECTOR))) void arch_tick_irq(void) {
	extern volatile sched_impl_t sched_p;

	/* Standard context switching logic. */
	arch_save_context();
	arch_acknowledge_tick_interrupt();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find the next logical thread in the sequence. */
	sched_impl_run();
	#if (CONFIG_DEBUG_MODE == 1)
		if (sched_p.sched_active_thread == NULL) panic(PANIC_EXPECT_FAIL, "NULL thread was scheduled");
	#endif

	/* Figure out when to service the next interrupt(s). */
	#if (CONFIG_USE_TICKLESS_IDLE == 1)

		/* If there is more than 1 thread, attention needs to be divided, so keep timeslicing. */
		if ((sched_p.state & SCHED_STATUS_THREAD_COUNT_MASK) > 1) arch_schedule_next_tick(ARCH_MS_TO_CYCLES(CONFIG_TICK_RATE_MS));
		else arch_suppress_tick_interrupt();

	/* Else service sleeping threads in software. */
	#else


		// check TA0R against the min every tick and perform wakeup() manually if true
		arch_schedule_next_tick(CONFIG_TICK_RATE_MS);
	#endif

	arch_restore_context();
}

__attribute__((naked, interrupt(ARCH_TIMEKEEPING_VECTOR))) void arch_time_irq(void) {
	extern volatile sched_impl_t sched_p;

	arch_enter_isr();
	switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {

		/* We shouldn't be able to trap into an interrupt without the hardware knowing. */
		case TA0IV_NONE:
			#if (CONFIG_DEBUG_MODE == 1)
				panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			#endif
			break;

		/* Whenever a wakeup happens. */
		case TA0IV_TACCR1:

			/* Necessary so we don't get in a loop here. */
			arch_acknowledge_wakeup_interrupt();

			/* First get the thread that needs to be awoken right now and add it back to the run queue. */
			volatile thread_impl_t *waker = sleep_queue_peek((sleep_queue_t *) &sched_p.sleep_mgr);
			sleep_queue_pop((sleep_queue_t *) &sched_p.sleep_mgr);
			sched_impl_register((thread_impl_t *) waker);

			/**
			 * Next find out who's next on the list, and if there is no one on the list, turn off the
			 * wakeup interrupt. Otherwise, configure the wakeup interrupt for the entry's stated wakeup time.
			 */
			volatile thread_impl_t *next_waker = sleep_queue_peek((sleep_queue_t *) &sched_p.sleep_mgr);
			if (next_waker == NULL) arch_suppress_wakeup_interrupt();
			else arch_schedule_next_wakeup(next_waker->sq_entry.wake_time);
			break;

		/* Unused for now, so these are all unexpected traps. */
		default:
			#if (CONFIG_DEBUG_MODE == 1)
				panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			#endif
			break;
	}
	arch_exit_isr();
}

/** @} */
