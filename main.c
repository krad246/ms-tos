#include "rtos.h"

volatile void *volatile sched_current_tcb;
volatile uint16_t sched_us_nesting = 1;

sched_flags_t sched_status_flags = SCHED_FLAGS_INITIAL_VALUE;

uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];

thread_t bober;

uint32_t bob = 0;

arch_reg_t arch_boot_sp = (arch_reg_t) NULL;

#define swap(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)

void clobber(void *arg) {
	uint32_t *bobp = (uint32_t *) arg;
	*bobp = *bobp + 1;

	arch_sched_end();
}
uintptr_t x;
void byte_refactor(uint8_t mem[4]) {
	volatile int x0, x1, x2, x3;

	x0 = mem[0];
	x1 = mem[1];
	x2 = mem[2];
	x3 = mem[3];

	mem[0] = x3;
	mem[1] = x2 << 4;
	mem[2] = x0;
	mem[3] = x1;
}

void byte_refactor2(uint8_t mem[4]) {
	uint16_t *top = (uint16_t *) mem;
	uint16_t *bot = top + 1;

	swap(*top, *bot, uint16_t);

	*((uint8_t *) top) <<= 4;
	*top = __swap_bytes(*top);
}

static inline void profile_start(void) {
	TA0CTL = MC_0 | TACLR;
	TA0CTL = MC_2 | TASSEL_2;
}

static inline void profile_end(void) {
	TA0CTL = MC_0;
}

void arch_setup_timer_interrupt(void) {

	WDTCTL = WDT_ADLY_16;
	SFRIE1 |= WDTIE;
}

void arch_disable_timer_interrupt(void) {
	SFRIE1 |= WDTIE;
}


volatile thread_t *sched_active_thread;

ISR(PORT1_VECTOR, on_button_press) {
	arch_enter_isr();
	P1IFG &= ~BIT1;
	arch_exit_isr();
}
volatile vtrr_client_t *frst_client;
vtrr_mgr_t manager;

volatile thread_t tcbs[6];
uint8_t sched_test_stacks[6][256];
volatile uint32_t run_counts[6] = { 0 };

void a(void *arg) {
	while (1) {
		run_counts[0]++;
		_low_power_mode_3();
		arch_yield();
	}
}

void b(void *arg) {
	while (1) {
		run_counts[1]++;
		_low_power_mode_3();
		arch_yield();
		arch_yield_higher();
	}
}

void c(void *arg) {
	while (1) {
		run_counts[2]++;
		_low_power_mode_3();
		arch_yield();
	}
}

void d(void *arg) {
	while (1) {
		run_counts[3]++;
		_low_power_mode_3();
		arch_yield();
	}
}

void e(void *arg) {
	while (1) {
		run_counts[4]++;
		_low_power_mode_3();
		arch_yield();
	}
}

void f(void *arg) {
	while (1) {
		run_counts[5]++;
		_low_power_mode_3();
		arch_yield();
	}
}


// design around a 60+ hz context switching interval with a 2nd timer for sleep queueing
// sleep will pull a node off of the run queue and push it to the sleep queue, which will maintain a list
// of tasks in sorted order of their wakeup times.
// CONFIG_SOFTWARE_TICK will bring the wakeup logic into the scheduler and prevent
// tickless mode, while  while CONFIG_HW_TICK will use another hardware timing module for tickless wakeup logic
// MSP430s MUST use the ACLK for the scheduler and the tick timer so 32768 Hz is a constant.
// 32768 Hz implies 32.768 ticks

uint16_t rc = 0;
uint32_t tt = 0;
void sched_run(void) {
	rc++;
	profile_start();
	vtrr_run(&manager);
	sched_active_thread = container_of(vtrr_active_client(&manager), thread_t, cli);
	profile_end();
	tt += TA0R;
}

void sched_yield_higher(void) {
	rc++;
	profile_start();
	vtrr_yield_higher(&manager);
	sched_active_thread = container_of(vtrr_active_client(&manager), thread_t, cli);
	profile_end();
	tt += TA0R;
}

/**
 * main.c
 */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	sched_active_thread = &bober;
	arch_setup_timer_interrupt();
	if (arch_interrupts_enabled()) {

	}
	P1DIR |= BIT0;                            // Set P1.0 to output direction
	P1OUT &= ~BIT0;
	P1REN |= BIT1;                            // Enable P1.1 internal resistance
	P1OUT |= BIT1;                            // Set P1.1 as pull-Up resistance
	P1IES &= ~BIT1;                           // P1.1 Lo/Hi edge
	P1IFG &= ~BIT1;                           // P1.1 IFG cleared
	P1IE |= BIT1;

	tcbs[0].sp = arch_init_stack(sched_test_stacks[0] + 256, a, run_counts[0]);
	tcbs[1].sp = arch_init_stack(sched_test_stacks[1] + 256, b, run_counts[1]);
	tcbs[2].sp = arch_init_stack(sched_test_stacks[2] + 256, c, run_counts[2]);
	tcbs[3].sp = arch_init_stack(sched_test_stacks[3] + 256, d, run_counts[3]);
	tcbs[4].sp = arch_init_stack(sched_test_stacks[4] + 256, e, run_counts[4]);
	tcbs[5].sp = arch_init_stack(sched_test_stacks[5] + 256, f, run_counts[5]);

	vtrr_init(&manager);

	for (int i = 0; i < 6; ++i) {
		vtrr_add(&manager, &tcbs[i].cli, i + 1);
	}

	vtrr_start(&manager);
	frst_client = vtrr_active_client(&manager);
	sched_active_thread = container_of(frst_client, thread_t, cli);

	arch_sched_start();

	while (1) {
		_low_power_mode_3();
		arch_yield();
	}
	return 0;
}
