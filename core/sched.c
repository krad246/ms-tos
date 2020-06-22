/*
 * sched.c
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "sched.h"

void sched_init(void) {
	irq_disable();

	sched_impl_init((sched_impl_mgr_t *) &sched_g.instance);
	sched_g.state = 0;
	sched_g.sched_active_thread = NULL;	// should point to idle thread by default, idle thread should always return itself as the next runnable thread

	irq_enable();
}

void sched_add(thread_t *new, unsigned int priority) {
	irq_lock();

	sched_impl_add((sched_impl_mgr_t *) &sched_g.instance, &new->cli, priority);

	irq_unlock();
}

void sched_register(thread_t *new) {
	irq_lock();

	sched_impl_register((sched_impl_mgr_t *) &sched_g.instance, &new->cli);

	irq_unlock();
}

void sched_deregister(thread_t *new) {
	irq_lock();

	sched_impl_deregister((sched_impl_mgr_t *) &sched_g.instance, &new->cli);

	irq_unlock();
}

void sched_reregister(thread_t *new, unsigned int priority) {
	irq_lock();

	sched_impl_reregister((sched_impl_mgr_t *) &sched_g.instance, &new->cli, priority);

	irq_unlock();
}

void sched_start(void) {
	irq_disable();

	sched_impl_start((sched_impl_mgr_t *) &sched_g.instance);
	sched_g.sched_active_thread = container_of(vtrr_entry(sched_g.instance.curr_cli), thread_t, cli);

	arch_sched_start();

	vtrr_entry(sched_g.instance.curr_cli);
	__sched_impl_active_client(vtrr, &sched_g.instance);
}

void sched_end(void) {
	irq_disable();

	sched_impl_end((sched_impl_mgr_t *) &sched_g.instance);
}
extern uint16_t rc ;
extern uint32_t tt;;
extern void profile_start(void);
extern void profile_end(void);

extern thread_t tcbs[6];
void sched_run(void) {
	rc++;
	profile_start();

	sched_impl_run((sched_impl_mgr_t *) &sched_g.instance);
	sched_g.sched_active_thread = container_of(vtrr_entry(sched_g.instance.curr_cli), thread_t, cli);

		profile_end();
		tt += TA0R;
}

void sched_yield(void) {
	rc++;
	profile_start();

	sched_impl_yield((sched_impl_mgr_t *) &sched_g.instance);
	sched_g.sched_active_thread = container_of(vtrr_entry(sched_g.instance.curr_cli), thread_t, cli);

	profile_end();
		tt += TA0R;
}

void sched_yield_higher(void) {
	rc++;
	profile_start();


	sched_impl_yield_higher((sched_impl_mgr_t *) &sched_g.instance);
	sched_g.sched_active_thread = container_of(vtrr_entry(sched_g.instance.curr_cli), thread_t, cli);

	profile_end();
		tt += TA0R;
}

sched_status_t sched_get_status(void) {
	irq_lock();

	sched_status_t curr_state = sched_g.state;

	irq_unlock();

	return curr_state;
}

void sched_set_status(sched_status_t status) {

}

void sched_clear_status(sched_status_t status) {

}

thread_t *sched_current_thread() {
	irq_lock();

	thread_t *curr = sched_g.sched_active_thread;

	irq_unlock();

	return curr;
}
