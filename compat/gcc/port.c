/*
 * port.c
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#include "port_defs.h"

void port_enable_interrupts(void) {
	__enable_interrupt();
}

void port_disable_interrupts(void) {
	__disable_interrupt();
}

port_flags_t port_get_interrupt_state(void) {
	return __get_SR_register() & GIE;
}

void port_set_interrupt_state(port_flags_t mask) {
	__bic_SR_register(GIE);
	__bis_SR_register(mask & GIE);
}

bool port_interrupts_enabled(void) {
	return (port_get_interrupt_state() == PORT_FLAG_INTERRUPTS_ENABLED);
}

void port_yield(void) {

	// 20-bit PC messes up trapframe
	#ifdef __MSP430X_LARGE__

		// top 4 bytes need to include RETI-formatted trapframe
		// need to swap top 2 bytes with bottom 2 bytes, then modify the top

		// atomic MOV saves SR at the point of entry
		__asm__ __volatile__("mov.b sr, 3(sp)");

		// once SR backed up, disable interrupts for the rest
		__disable_interrupt();

		// r12 used as a temp variable - prevent it from getting clobbered
		__asm__ __volatile__("pushx.a r12");

		// shifts upper 4 bits of PC to higher nibble at sp[2]
		__asm__ __volatile__("mov.b 6(sp), r12");
		__asm__ __volatile__("rpt #4");
		__asm__ __volatile__("rlax.b r12");
		__asm__ __volatile__("mov.b r12, 6(sp)");

		// swap PC + SR bytes for RETI format
		__asm__ __volatile__("swpb 6(sp)");

		// swap PC[0:15] and PC + SR words for RETI format
		__asm__ __volatile__("mov.w 6(sp), r12");
		__asm__ __volatile__("mov.w 4(sp), 6(sp)");
		__asm__ __volatile__("mov.w r12, 4(sp)");

		// restore r12 for context switch
		__asm__ __volatile__("popx.a r12");
	#else
		__asm__ __volatile__("push sr");
		__disable_interrupt();
	#endif

	port_save_context();
	sched_run();
	port_restore_context();
}

static void port_create_irq_frame(port_iframe_t *ptr_frame,
		thread_fn_t ptr_xcode, port_flags_t cpu_flags) {

	#ifdef __MSP430X_LARGE__
		const port_reg_t addr_runnable = (port_reg_t) ptr_xcode;

		const uint16_t addr_runnable_l = addr_runnable & 0x0000FFFF;
		const uint16_t addr_runnable_h = (addr_runnable & 0x000F0000) >> 16;

		ptr_frame->words[1] = addr_runnable_l;
		ptr_frame->bytes[0] = (uint8_t) cpu_flags;
		ptr_frame->bytes[1] = (uint8_t) (addr_runnable_h << 4);
	#else
		ptr_frame->words[1] = (uint16_t) ptr_xcode;
		ptr_frame->words[0] = (uint8_t) cpu_flags;
	#endif
}

port_reg_t *port_init_stack(port_reg_t *ptr_stack_top, thread_fn_t ptr_xcode, void *ptr_fn_args) {
	ptr_stack_top = (port_reg_t *) (((uint8_t *) ptr_stack_top) - sizeof(port_iframe_t));
	port_create_irq_frame((port_iframe_t *) ptr_stack_top, ptr_xcode, GIE);

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0xeeee; 	// r15

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0xdddd;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0xcccc;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) ptr_fn_args;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0xbbbb;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0xaaaa;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x9999;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x8888;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x7777;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x6666;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x5555;

	ptr_stack_top--;
	*ptr_stack_top = (port_reg_t) 0x4444;	// r4

	return ptr_stack_top;
}

static void port_setup_timer_interrupt(void) {
	app_setup_timer_interrupt();
}

void port_start_sched(void) {
	// save context for spawning spot
	// setup timer interrupt
	port_setup_timer_interrupt();
	// schedule a new function
	// restore context into the function

	// barrier -------------------------

	// clean up and deallocate everything
	// ret
}

void port_end_sched(void) {
	// point the stack pointer to the spawning stack top
	// restore context
}

static __attribute__((naked, interrupt(CONFIG_TICK_VECTOR))) void sched_tick_irq(void) {
	port_save_context();

	sched_run();

	port_restore_context();
}
