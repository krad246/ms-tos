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

#include "irq.h"
#include "thread.h"
#include "sleep_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread thread_t;

typedef unsigned int sched_status_t;

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

void sched_add(volatile thread_t *new, volatile unsigned int priority);

void sched_register(volatile thread_t *new);

void sched_deregister(volatile thread_t *new);

void sched_reregister(volatile thread_t *new, volatile unsigned int priority);

void sched_start(void);

void sched_end(void);

void sched_run(void);

void sched_yield(void);

void sched_yield_higher(void);

sched_status_t sched_get_status(void);

void sched_set_status(sched_status_t status);

void sched_clear_status(sched_status_t status);

thread_t *sched_current_thread(void);
unsigned int sched_thread_count(void);
#include "sched_impl.h"
struct vtrr_{
	sched_impl_client_t run;
	sched_impl_client_t done;
};
void sched_register_cb(void (*cb)(void *arg), void *params);
void sched_task_exit(void);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHED_H_ */
