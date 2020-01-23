/**
\file pqueue.h
\brief A generic priority queue.
*/

#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdlib.h>
#include <stdint.h>

/*
A very simple priority queue.

Example:
PQUEUE(test, int, count)

Creates:
struct pqueue_test {...};
static inline void queue_test_init(struct queue_test *) {...}
static inline int queue_test_push(struct queue_test *, int *) {...}
static inline int queue_test_pop(struct queue_test *, int *) {...}

API:
queue_*_init initializes a queue
queue_*_push pushes an item onto the queue, returns 0 if successful, not 0 if fail
queue_*_pop pops an item from the queue, returns 0 if successful, not 0 if fail
queue_*_foreach takes a function pointer and pointer to some context and for each
    element in the queue calls the function with a pointer to that element. If the
    returns zero queue_*_foreach will continue processing the rest of the items, if
    the function returns non zero then queue_*_foreach will not process any more items.

*/

/**
\brief Generates the queue api
\param name a name for the api with the given type and size
\param type the type of data to store in the queue
\param size the max number of data elements
*/
#define PQUEUE(name, type, size)                                                        \
struct pqueue_##name##_node {															\
	int priority;																		\
	type data;																			\
};																						\
struct pqueue_##name {                                                                  \
    struct pqueue_##name##_node storage[size];                                          \
    /*number of items in the queue*/                                                    \
    size_t count;                                                                       \
};                                                                                      \
static inline void pqueue_##name##_init(volatile struct pqueue_##name *pq) {            \
	pq->count = 0;																		\
	memset((void *) pq->storage, 0, size * sizeof(struct pqueue_##name##_node));		\
}                                                                                       \
static inline int pqueue_##name##_push(volatile struct pqueue_##name *pq,               \
									  size_t priority,									\
                                      const volatile type *item) {                      \
	if (pq->count + 1 >= size) {														\
		return -1;																		\
	} else {																			\
		size_t i = pq->count + 1;														\
		size_t j = i / 2;																\
		while (i > 1 && pq->storage[j].priority > priority) {							\
			pq->storage[i] = pq->storage[j];											\
			i = j;																		\
			j = j / 2;																	\
		}																				\
		pq->storage[i].priority = priority;												\
		pq->storage[i].data = *item;													\
		pq->count++;																	\
		return 0;																		\
	}																					\
}                                                                                       \
static inline int pqueue_##name##_pop(volatile struct pqueue_##name *pq,                \
                                     volatile type *item) {                             \
    if (!pq) {																			\
        return -1;																		\
    } else {																			\
		*item = pq->storage[1].data;													\
		pq->storage[1] = pq->storage[pq->count];										\
		pq->count--;																	\
		size_t i = 1;																	\
		size_t j, k;																	\
		while (i != pq->count + 1) {													\
			k = pq->count + 1;															\
			j = 2 * i;																	\
			if (j <= pq->count && 														\
				pq->storage[j].priority < pq->storage[k].priority) {					\
				k = j;																	\
			}																			\
			if (j + 1 <= pq->count && 													\
				pq->storage[j + 1].priority < pq->storage[k].priority) {				\
				k = j + 1;																\
			}																			\
			pq->storage[i] = pq->storage[k];											\
			i = k;																		\
		}																				\
    	return 0;																		\
	}																					\
}                                                                                       \
static inline int pqueue_##name##_peek(volatile struct pqueue_##name *pq,              	\
                                     volatile type *item) {                             \
    if (pq->count > 0) {                                                                \
    	*item = pq->storage[1].data;													\
        return 0;                                                                       \
    } else {                                                                            \
        return -1;                                                                      \
    }                                                                                   \
}																						\
static inline size_t pqueue_##name##_count(const volatile struct pqueue_##name *pq) {   \
    return pq->count;                                                                   \
}                                                                                       \

#endif // PQUEUE_H

