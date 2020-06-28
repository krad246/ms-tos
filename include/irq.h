/*
 * irq.h
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_IRQ_H_
#define INCLUDE_IRQ_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int irq_lock_t;

void irq_disable(void);
void irq_enable(void);

void irq_lock(void);
void irq_unlock(void);

bool irq_is_in(void);
bool irq_locked(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_IRQ_H_ */
