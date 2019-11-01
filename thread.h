//
// Created by gta on 10/24/19.
//

#ifndef RTOS_THREAD_H
#define RTOS_THREAD_H

#include <defines.h>

/**
 * MSP430 Trapframe
 * - Generated on an interrupt
 * - Contains PC + SR
 */

typedef union __attribute__((packed)) {
	uint32_t value;
	uint16_t words[2];

	#if defined(RTOS_MSP430X_TRAPFRAME__)
		struct {
			unsigned int sr : 12;
			unsigned int pc_high : 4;
			unsigned int pc_low : 16;
		};
	#else
		struct {
			unsigned int sr : 16;
			unsigned int pc : 16;
		};
	#endif
} trapframe_t;

/**
 * Thread Context
 * - Contains register states
 * - Contains trapframe for RETI from scheduler interrupt
 */

struct context {
  word_t r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, sp;
  trapframe_t tf;
};

/**
 * Thread object
 * - Contains thread context
 * - Contains stack
 */

typedef uint8_t tid_t;

struct thread {

	// Thread context
	union {
		struct context ctx;
		word_t regs[12];
	};

	// Thread ID
	tid_t tid;

	struct thread *next;
	bool available;
};

trapframe_t trapframe(word_t pc, word_t sr);

word_t thread_get_sp(struct thread *this);
void thread_set_sp(struct thread *this, word_t new);

#endif //RTOS_THREAD_H
