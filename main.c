#include <os.h>

/**
 * main.c
 */

int foo(void *arg) {
	P1DIR |= BIT0;
	while (1) {
		P1OUT ^= BIT0;
		__delay_cycles(50000);
	}
}

int bar(void *arg) {
	P4DIR |= BIT7;
	while (1) {
		P4OUT ^= BIT7;
		__delay_cycles(100000);
	}
}


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	os_init();
	os_thread_create(foo, NULL);
	os_thread_create(bar, NULL);
	os_launch();

	return 0;
}
