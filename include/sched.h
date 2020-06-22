/*
 * sched.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_H_
#define INCLUDE_SCHED_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "port_config.h"

#include "sched_impl.h"

#include "irq.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread thread_t;

typedef unsigned int sched_status_t;

typedef struct sched {
	sched_impl_mgr_t instance;

	sched_status_t state;
	thread_t *sched_active_thread;

	#ifdef CONFIG_USE_KERNEL_STACK
		uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
	#endif

	#ifdef CONFIG_USE_TICK_HOOK
		void (*sched_tick_cb)(void *arg);
		void *tick_hook_params;
	#endif

} sched_t;

#define STATUS_IN_IRQ 1
#define STATUS_CONTEXT_SWITCH_REQUEST 2

extern volatile sched_t sched_g;

typedef enum {
    STATUS_STOPPED,
    STATUS_ZOMBIE,
    STATUS_SLEEPING,
    STATUS_MUTEX_BLOCKED,
    STATUS_RECEIVE_BLOCKED,
    STATUS_SEND_BLOCKED,
    STATUS_REPLY_BLOCKED,
    STATUS_FLAG_BLOCKED_ANY,
    STATUS_FLAG_BLOCKED_ALL,
    STATUS_MBOX_BLOCKED,
    STATUS_COND_BLOCKED,
    STATUS_RUNNING,
    STATUS_PENDING,
    STATUS_NUMOF
} thread_status_t;

void sched_init(void);

void sched_add(thread_t *new, unsigned int priority);

void sched_register(thread_t *new);

void sched_deregister(thread_t *new);

void sched_reregister(thread_t *new, unsigned int priority);

void sched_start(void);

void sched_end(void);

void sched_run(void);

void sched_yield(void);

void sched_yield_higher(void);

sched_status_t sched_get_status(void);

void sched_set_status(sched_status_t status);

void sched_clear_status(sched_status_t status);

thread_t *sched_current_thread(void);

void sched_register_cb(void (*cb)(void *arg), void *params);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHED_H_ */
