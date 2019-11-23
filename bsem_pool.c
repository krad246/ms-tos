/*
 * bsem_pool.c
 *
 *  Created on: Nov 22, 2019
 *      Author: krad2
 */

#include <bsem_pool.h>

void bsem_pool_init(bsem_pool_t *sem) {
	sem->_valid = 0;
	sem->_sem_pt = 0;
	sem->_sems = 0xF;
}

bool bsem_pool_empty(bsem_pool_t *sem) {
	return sem->_valid == 0 && sem->_sems == 0xF;
}

uint8_t bsem_pool_size(bsem_pool_t *sem) {
	if (bsem_pool_empty(sem)) return 0;
	else return 1 + sem->_valid;
}

bool bsem_pool_full(bsem_pool_t *sem) {
	return bsem_pool_size(sem) == 4;
}

int8_t bsem_pool_alloc(bsem_pool_t *sem) {
	if (bsem_pool_full(sem)) return -1;

	if (bsem_pool_empty(sem)) {
		sem->pool = 1;
		return 0;
	}

	sem->_valid++;
	sem->_sems |= (1u << sem->_valid);
	return sem->_valid;
}

static unsigned delete_bits_and_concat(unsigned x, unsigned n, unsigned k) {
	const uint8_t hi_bits_to_clear = ~0u << n;
	const uint8_t saved_low_bits = x & ~hi_bits_to_clear;
	const uint8_t hi_bits_to_save = ~0u << (n + k);
	const uint8_t saved_hi_bits = x & hi_bits_to_save;

	const uint8_t concat = (saved_hi_bits >> k) | saved_low_bits;
	return concat;
}

int8_t bsem_pool_free(bsem_pool_t *sem, uint8_t idx) {
	if (bsem_pool_empty(sem)) return -1;
	if (idx > sem->_valid) return -1;

	const uint8_t pop = delete_bits_and_concat(sem->_sems, idx, 1) & 0xF;
	if (sem->_valid > 1) sem->_sems = 0x8 | pop;
	else if (sem->_valid == 0) sem->_sems = 0xF;
	else sem->_sems = pop;

	if (sem->_valid != 0) sem->_valid--;

	return 0;
}

int8_t bsem_pool_get_active_sem(bsem_pool_t *sem) {
	if (bsem_pool_empty(sem)) return -1;
	return sem->_sem_pt;
}

int8_t bsem_pool_change_active_sem(bsem_pool_t *sem, uint8_t idx) {
	if (idx > sem->_valid) return -1;

	sem->_sem_pt = idx;
	return 0;
}

int8_t bsem_pool_active_sem_get_val(bsem_pool_t *sem) {
	const uint8_t idx = sem->_sem_pt;
	if (idx > sem->_valid) return -1;

	const uint8_t shift_val = (1u << idx);
	return (sem->pool & shift_val) >> idx;
}

int8_t bsem_pool_active_sem_set_val(bsem_pool_t *sem, uint8_t val) {
	if (val > 1) return -1;

	const uint8_t idx = sem->_sem_pt;
	const uint8_t clr_val = (1u << idx);

	const uint8_t cleared = sem->_sems & ~clr_val;
	const uint8_t modified = cleared | (val << idx);

	sem->_sems = modified;
	return 0;
}

void bsem_pool_wait(bsem_pool_t *sem) {
	_start_critical();
	if (bsem_pool_active_sem_get_val(sem) == 0) {
		// blocking code
		// - unlink from run queue
		// - add to wait queue

		_end_critical();
		os_yield();
	} else {
		bsem_pool_active_sem_set_val(sem, 0);
	}
	_end_critical();
}

void bsem_pool_signal(bsem_pool_t *sem) {
	_start_critical();
	if (bsem_pool_active_sem_get_val(sem) == 0) {
//		if (runQueue.isEmpty()) {
//			bsem_pool_active_set_val(sem, 1)
//		} else {
//			thrd = runQueue.pop();
//			thrd.wakeup();
//		}
	}
	_end_critical();
}
