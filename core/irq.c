/*
 * irq.c
 *
 *  Created on: Jun 22, 2020
 *      Author: krad2
 */

#include "irq.h"
#include "rtos.h"

void irq_disable(void) {
	arch_disable_interrupts();
}

void irq_enable(void) {
	arch_enable_interrupts();
}

void irq_lock(void) {
	if (arch_interrupts_enabled()) {
		arch_disable_interrupts();
		sched_g.sched_active_thread->cs_lock = 0;
	}

	sched_g.sched_active_thread->cs_lock++;
}

void irq_unlock(void) {
	sched_g.sched_active_thread->cs_lock--;
	if (sched_g.sched_active_thread->cs_lock == 0) {
		sched_g.sched_active_thread->cs_lock = 1;
		arch_enable_interrupts();
	}
}

bool irq_is_in(void) {

	irq_lock();
	bool isr_state = sched_get_status() & STATUS_IN_IRQ;
	irq_unlock();

	return isr_state;
}

