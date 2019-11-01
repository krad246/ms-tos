/*
 * thread.c
 *
 *  Created on: Oct 31, 2019
 *      Author: krad2
 */

#include <thread.h>

trapframe_t trapframe(word_t pc, word_t sr) {
	trapframe_t tf;
	#if defined(RTOS_MSP430X_TRAPFRAME__)
		unsigned int pc_high_ = (pc & 0xF0000) >> 16;
		unsigned int pc_low_ = pc & 0xFFFF;
		unsigned int sr_ = sr & 0xFFF;

		tf.pc_high = pc_high_;
		tf.pc_low = pc_low_;
		tf.sr = sr_;
	#else
		tf.pc = pc & 0xFFFF;
		tf.sr = sr & 0xFF;
	#endif

	return tf;
}
