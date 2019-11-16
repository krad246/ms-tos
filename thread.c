#include <thread.h>

int hw_stackframe_init(hw_stackframe_t *frame, pc_t pc, word_t sr) {
	if (frame == 0) return -1;	// InvalidArgument
	if (pc == 0) return -1;	// InvalidArgument

	#if defined(__LARGE_CODE_MODEL__)
		unsigned int pc_high_ = (pc & 0xF0000) >> 16;	// PC[19:16]
		unsigned int pc_low_ = pc & 0xFFFF;				// PC[15:0]
		unsigned int sr_ = sr & 0xFFF;					// SR

		frame->pc_high = pc_high_;
		frame->pc_low = pc_low_;
		frame->sr = sr_;
	#else
		frame->pc = pc & 0xFFFF;
		frame->sr = sr & 0xFF;
	#endif

	return 0;
}

int sw_stackframe_init(sw_stackframe_t *frame, word_t sp, word_t arg) {
	if (frame == 0) return -1; // InvalidArgument
	if (sp == 0) return -1; // InvalidArgument

	// Zero-initialize the stack, set the stack pointer to the location of the HW stack frame, and set the argument pointer
	memset(frame, 0, sizeof(sw_stackframe_t));
	frame->sp = sp;
	frame->r12 = arg;

	return 0;
}

const uint8_t *thrd_stack_base(thrd_t *this) {
	const uint16_t sz = sizeof(this->stack) / sizeof(*this->stack);
	const uint16_t *stack_top = this->stack;
	return (const uint8_t *) (stack_top + sz);
}

int thrd_stack_init(thrd_t *this, pc_t pc) {
	if (this == NULL) return -1; // InvalidArgument
	if (pc == 0) return -1; // InvalidArgument

	memset(this->stack, 0, sizeof(this->stack));

	const uint8_t *stack_bottom = thrd_stack_base(this);

	// need to also add thrd_exit return address here
	const uint16_t hw_stackframe_offset = sizeof(hw_stackframe_t);
	uint8_t *hw_stackframe_loc = (uint8_t *) (stack_bottom - hw_stackframe_offset);

	int status;
	status = hw_stackframe_init((hw_stackframe_t *) hw_stackframe_loc, pc, GIE);

	return status;
}

int thrd_context_init(thrd_t *this, word_t sp, word_t arg) {
	if (this == NULL) return -1;
	if (sp == 0) return -1;

	int status;
	status = sw_stackframe_init(&this->context, sp, arg);

	return status;
}

int thrd_init(thrd_t *this, int (*routine)(void *), void *arg) {
	int status;

	status = thrd_stack_init(this, (pc_t) routine);
	if (status < 0) return status;

	const uint8_t *stack_bottom = thrd_stack_base(this);
	const uint16_t hw_stackframe_offset = sizeof(hw_stackframe_t);
	uint8_t *hw_stackframe_loc = (uint8_t *) (stack_bottom - hw_stackframe_offset);

	status = thrd_context_init(this, (word_t) hw_stackframe_loc, (word_t) arg);

	return status;
}

int thrd_create(int (*routine)(void *), void *arg) {
	return task_create(routine, arg);
}
