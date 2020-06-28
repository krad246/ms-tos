#include "rtos.h"
#include "msp430.h"

#define NUM_THREADS		6
#define STACK_SIZE		256

volatile uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
volatile thread_t tcbs[NUM_THREADS];
volatile uint8_t sched_test_stacks[NUM_THREADS][STACK_SIZE];
volatile uint32_t run_counts[NUM_THREADS] = { 0 };

/*-----------------------------------------------------------*/

volatile uint16_t rc = 0;
volatile uint32_t tt = 0;

void profile_start(void) {
	TA1CTL = MC_0 | TACLR;
	TA1CTL = MC_2 | TASSEL_2;
}

void profile_end(void) {
	TA1CTL = MC_0;
	tt += TA1R;
	rc++;
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
		sched_sleep(500);
		P1OUT ^= BIT1;
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

/*-----------------------------------------------------------*/

/**
 * main.c
 */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	P1DIR |= (BIT0 | BIT1);                            // Set P1.0 to output direction
	P1OUT &= ~(BIT0 | BIT1);

	PM5CTL0 &= ~LOCKLPM5;       //  Enable GPIOs

	tcbs[0].base.sp = arch_init_stack(sched_test_stacks[0] + 256, a, run_counts[0]);
	tcbs[1].base.sp = arch_init_stack(sched_test_stacks[1] + 256, b, run_counts[1]);
	tcbs[2].base.sp = arch_init_stack(sched_test_stacks[2] + 256, c, run_counts[2]);
	tcbs[3].base.sp = arch_init_stack(sched_test_stacks[3] + 256, d, run_counts[3]);
	tcbs[4].base.sp = arch_init_stack(sched_test_stacks[4] + 256, e, run_counts[4]);
	tcbs[5].base.sp = arch_init_stack(sched_test_stacks[5] + 256, f, run_counts[5]);

	__disable_interrupt();
	sched_init();

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
