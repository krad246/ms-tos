#include "rtos.h"

#define NUM_THREADS		6
#define STACK_SIZE		256

uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
arch_reg_t arch_boot_sp = (arch_reg_t) NULL;

volatile sched_t sched_g;

volatile thread_t tcbs[NUM_THREADS];
uint8_t sched_test_stacks[NUM_THREADS][STACK_SIZE];
volatile uint32_t run_counts[NUM_THREADS] = { 0 };

/*-----------------------------------------------------------*/

void profile_start(void) {
	TA0CTL = MC_0 | TACLR;
	TA0CTL = MC_2 | TASSEL_2;
}

void profile_end(void) {
	TA0CTL = MC_0;
}

void arch_setup_timer_interrupt(void) {
	WDTCTL = WDT_ADLY_1_9;
	SFRIE1 |= WDTIE;
}

void arch_disable_timer_interrupt(void) {
	WDTCTL = WDTPW | WDTHOLD;
}

/*-----------------------------------------------------------*/

ISR(PORT1_VECTOR, on_button_press) {
	arch_enter_isr();
	P1OUT ^= BIT0;
	arch_exit_isr();
}

/*-----------------------------------------------------------*/

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

/*-----------------------------------------------------------*/

// design around a 60+ hz context switching interval with a 2nd timer for sleep queueing
// sleep will pull a node off of the run queue and push it to the sleep queue, which will maintain a list
// of tasks in sorted order of their wakeup times.
// CONFIG_SOFTWARE_TICK will bring the wakeup logic into the scheduler and prevent
// tickless mode, while  while CONFIG_HW_TICK will use another hardware timing module for tickless wakeup logic
// MSP430s MUST use the ACLK for the scheduler and the tick timer so 32768 Hz is a constant.
// 32768 Hz implies 32.768 ticks

uint16_t rc = 0;
uint32_t tt = 0;

//void sched_run(void) {
//	rc++;
//	profile_start();
//	irq_lock();
//	sched_run();
//	irq_unlock();
//	profile_end();
//	tt += TA0R;
//}
//
//void sched_yield_higher(void) {
//	rc++;
//	profile_start();
//	sched_yield_higher();
//	profile_end();
//	tt += TA0R;
//}

/*-----------------------------------------------------------*/

/**
 * main.c
 */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

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

	sched_init();
	sched_g.sched_active_thread = &tcbs[0];

	for (int i = 0; i < 6; ++i) {
		tcbs[i].cs_lock = 1;
		sched_add(&tcbs[i], i + 1);
	}

	sched_start();

	while (1) {
		_low_power_mode_3();
		arch_yield();
	}
	return 0;
}
