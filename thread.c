#include <thread.h>
#include <semaphore.h>
#include <os.h>

void hw_stackframe_init(hw_stackframe_t *frame, pc_t pc, word_t sr) {
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
}

static void sw_stackframe_init(sw_stackframe_t *frame, word_t arg) {
	// Zero-initialize the stack, set the stack pointer to the location of the HW stack frame, and set the argument pointer
	memset(frame, 0, sizeof(sw_stackframe_t));
	frame->r12 = arg;
}

static const uint8_t *thrd_stack_base(thrd_t *this) {
	const uint16_t sz = sizeof(this->stack) / sizeof(*this->stack);
	const uint16_t *stack_top = this->stack;
	return (const uint8_t *) (stack_top + sz);
}

static int thrd_stack_init(thrd_t *this, pc_t pc, word_t arg) {
	if (this == NULL) return -1; // InvalidArgument
	if (pc == 0) return -1; // InvalidArgument

	memset(this->stack, 0, sizeof(this->stack));

	const uint8_t *stack_base = (uint8_t *) thrd_stack_base(this);

	const size_t hw_sf_offs = sizeof(hw_stackframe_t);
	const size_t sw_sf_offs = sizeof(sw_stackframe_t);

	const uint8_t *hw_sf_start = stack_base - hw_sf_offs;
	const uint8_t *sw_sf_start = stack_base - (hw_sf_offs + sw_sf_offs);

	hw_stackframe_init((hw_stackframe_t *) hw_sf_start, pc, GIE);
	sw_stackframe_init((sw_stackframe_t *) sw_sf_start, arg);
	this->sp = (word_t) sw_sf_start;

	return 0;
}

static void thrd_kill(void) {
	extern void os_task_kill(void);
	os_task_kill();
}

static void thrd_exit(int ret_code) {
	extern void os_task_exit(int ret_code);
	os_task_exit(ret_code);
	thrd_kill();
}

int thrd_init(thrd_t *this, int (*routine)(void *), void *arg, size_t priority) {
	
	int status;

	/**
	 * Set up thread stack
	 */

	status = thrd_stack_init(this, (pc_t) routine, (word_t) arg);
	if (status < 0) return status;

	/**
	 * Add exit handler to thread
	 */

	this->ret_addr = (pc_t) thrd_exit;

	/**
	 * Initialize the semaphore that other threads can wait on
	 */

	sem_init(&this->join_sem, 0);

	/**
	 * Set the priority levels for WRR scheduling
	 */

	this->base_prio = priority;
	this->fixed_prio = priority;
	this->working_prio = priority;

	/**
	 * Mark as active
	 */
	this->state = ACTIVE;

	return status;
}

int thrd_create(int (*routine)(void *), void *arg, size_t priority) {
	if (routine == NULL) return -1;
	if (priority == 0) return -1;

	int status = 0;

	start_critical();

	extern os_task_t *os_task_create(int (*routine)(void *), void *arg);
	os_task_t *thrd = os_task_create(routine, arg);

	if (thrd) thrd_init((thrd_t *) thrd, routine, arg, priority);
	else status = -1;

	end_critical();

	return status;
}

void thrd_yield(void) {
	extern void os_task_yield(void);
	os_task_yield();
}

void thrd_sleep(size_t ticks) {
	extern void os_task_sleep(thrd_t *caller, size_t ticks);
	thrd_t *caller = (thrd_t *) thrd_current();
	os_task_sleep(caller, ticks);
}

const thrd_t *thrd_current(void) {
	extern const thrd_t *os_task_current(void);
	return os_task_current();
}

int thrd_equal(thrd_t *lhs, thrd_t *rhs) {
	return lhs == rhs;
}

void thrd_join(thrd_t *other, int *const ret_code) {
	extern int os_task_join(thrd_t *other, int *const ret_code);
	os_task_join(other, ret_code);
}

int thrd_detach(thrd_t *thr) {
	extern int os_task_detach(thrd_t *thr);
	return os_task_detach(thr);
}
