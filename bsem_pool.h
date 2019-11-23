/*
 * bsem_pool.h
 *
 *  Created on: Nov 22, 2019
 *      Author: krad2
 */

#ifndef BSEM_POOL_H_
#define BSEM_POOL_H_

#include <definitions.h>
#include <os.h>

// Packed binary semaphore 'pool' object
// Fits into an 8-bit int
typedef union __attribute__((packed)) {
	uint8_t pool;
	struct {
		uint8_t _sems : 4;
		uint8_t _valid : 2;
		uint8_t _sem_pt : 2;
	};
} bsem_pool_t;

void bsem_pool_init(bsem_pool_t *sem);

bool bsem_pool_empty(bsem_pool_t *sem);
bool bsem_pool_full(bsem_pool_t *sem);

uint8_t bsem_pool_size(bsem_pool_t *sem);

int8_t bsem_pool_alloc(bsem_pool_t *sem);
int8_t bsem_pool_free(bsem_pool_t *sem, uint8_t idx);

int8_t bsem_pool_get_active_sem(bsem_pool_t *sem);
int8_t bsem_pool_change_active_sem(bsem_pool_t *sem, uint8_t idx);

int8_t bsem_pool_active_sem_get_val(bsem_pool_t *sem);
int8_t bsem_pool_active_sem_set_val(bsem_pool_t *sem, uint8_t val);

void bsem_pool_wait(bsem_pool_t *sem);
void bsem_pool_signal(bsem_pool_t *sem);

// TODO:
// BUILD a 16 bit version of the form
// 4 : 4 : 8
//

#endif /* BSEM_POOL_H_ */
