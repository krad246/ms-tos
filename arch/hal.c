/*
 * hal.c
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#include "hal.h"

/**
 * @name Interrupt control wrappers
 * @{
 */

#define ARCH_FLAG_INTERRUPTS_ENABLED ((arch_flags_t) GIE)

void arch_disable_interrupts(void) {
	__disable_interrupt();	/* defined in msp430.h */
}

void arch_enable_interrupts(void) {
	__enable_interrupt();	/* defined in msp430.h */
}

void arch_set_interrupt_state(arch_flags_t mask) {
	/* erase the interrupt enable bit, then set it if appropriate */
	__bic_SR_register(GIE);
	__bis_SR_register(mask & GIE);
}

arch_flags_t arch_get_interrupt_state(void) {
	/* nothing but the interrupt enable bit GIE matters */
	return __get_SR_register() & GIE;
}

bool arch_interrupts_enabled(void) {
	return (arch_get_interrupt_state() == ARCH_FLAG_INTERRUPTS_ENABLED);
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name MSP430 ISA-specific stack initialization.
 * @{
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

arch_reg_t *arch_init_stack(arch_reg_t *stack_top, thread_fn_t xcode, void *fn_args) {

	/* Precondition: stack_top points to the bottom of the task stack. */

	/**
	 * Preallocate a constant number of bytes for the interrupt frame,
	 * then create the frame centered on that memory space at stack_top.
	 */
	const uint8_t *stack_base = (uint8_t *) stack_top;
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

	stack_top--;						/* Preallocate R12 */
	*stack_top = (arch_reg_t) fn_args;	/* Preload R12 to fn_args */

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
extern arch_reg_t arch_boot_sp;
void __attribute__((noinline)) arch_sched_start(void) {


	arch_disable_interrupts();

	/* Back up the context before booting up the kernel so that we can return later. */
	arch_save_regs();
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a sp, %0" : "=r"(arch_boot_sp));
	#else
		__asm__ __volatile__("mov.w sp, %0" : "=r"(arch_boot_sp));
	#endif

//	sched_init();
	sched_run();

	arch_setup_timer_interrupt();
	arch_restore_context();

	// barrier -------------------------

	// clean up and deallocate everything
	// ret
}

void __attribute__((naked)) arch_sched_end(void) {
	arch_disable_timer_interrupt();
	arch_disable_interrupts();

	/* Reload the pre-kernel SP because we are going to use that to exit all tasks. */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a %0, sp" : : "m"(arch_boot_sp));
	#else
		__asm__ __volatile__("mov.w %0, sp" : : "m"(arch_boot_sp));
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

static inline void __attribute__((always_inline)) arch_context_stack(void) {

	/**
	 * We need to emulate the stacking behavior of a preemptive tick.
	 * Since a tick pushes the PC + SR on the stack, we also need to do that.
	 * As this is a function call, we have to manually push the SR.
	 * The rest reads like a standard context switch.
	 */

	/**
	 * The 20-bit trapframe is bizarre as previously outlined,
	 * so additional work needs to be done to format a normal function address into
	 * something RETI can work with.
	 */
	#ifdef __MSP430X_LARGE__

		/* Save the SR */
		__asm__ __volatile__("mov.b sr, 3(sp)");

		/* PC + SR is stacked so interrupts are disabled */
		__disable_interrupt();

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
	#else
		__asm__ __volatile__("push sr");
		__disable_interrupt();
	#endif
}

void __attribute__((noinline)) arch_yield(void) {

	/* Manual context stacking. */
	arch_context_stack();

	/** Standard context switching logic. */
	arch_save_context();
	sched_run();
	arch_restore_context();
}

/** @} */

void __attribute__((noinline)) arch_yield_higher(void) {

	/* Manual context stacking. */
	arch_context_stack();

	/** Standard context switching logic. */
	arch_save_context();
	sched_yield_higher();
	arch_restore_context();
}

/**
 * @brief Scheduler preemption tick. Invokes sched_run() to distribute time slices.
 */

__attribute__((naked, interrupt(CONFIG_TICK_VECTOR))) void sched_tick_irq(void) {
	arch_save_context();
	sched_run();
	arch_restore_context();
}