/*
 * thread_impl.c
 *
 *  Created on: Jun 26, 2020
 *      Author: krad2
 */

#include <al.h>
#include "thread_impl.h"

void thread_impl_init(thread_impl_t *me, void *sp, thread_fn_t runnable, void *args) {
	me->sp = (void *) al_init_stack(sp, runnable, args);
}
