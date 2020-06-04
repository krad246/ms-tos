/*
 * sched_flags.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_FLAGS_H_
#define INCLUDE_SCHED_FLAGS_H_

#include <stdint.h>

#include "port_config.h"
#include "panic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCHED_FLAG_LOCKED (1u << 15)
#define SCHED_FLAG_IN_IRQ (1u << 14)
#define SCHED_FLAG_CONTEXT_SWITCH_REQUESTED (1u << 13)

#define SCHED_STATUS_MASK (SCHED_FLAG_LOCKED | SCHED_FLAG_IN_IRQ | SCHED_FLAG_CONTEXT_SWITCH_REQUESTED)
#define SCHED_CRITICAL_NESTING_MASK (~SCHED_STATUS_MASK)

#define SCHED_CS(x) ((x) & SCHED_CRITICAL_NESTING_MASK)
#define SCHED_STATUS(x) ((x) & SCHED_STATUS_MASK)

#define SCHED_FLAGS_INITIAL_VALUE (1)

typedef uint16_t sched_flags_t;

void sched_flags_cs_set(sched_flags_t *ptr_flags, sched_flags_t mask);

sched_flags_t sched_flags_cs_get(sched_flags_t *ptr_flags);

void sched_flags_cs_increment(sched_flags_t *ptr_flags);

void sched_flags_cs_decrement(sched_flags_t *ptr_flags);

void sched_set_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask);

void sched_clear_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask);

void sched_assign_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHED_FLAGS_H_ */
