#include <thread.h>
#include <stdarg.h>
#include <os.h>
#include <semaphore.h>
#include <mutex.h>

void uart_putc(unsigned);
void uart_puts(char *);
void uart_send_byte(unsigned char byte);
void uart_printf(char *format, ...);
void uart_init(void);

/**
 * main.c
 */

volatile thrd_t *bob;
sem_t uart_sem;
mtx_t uart_mtx;

int foo(void *arg) {
	P1DIR |= BIT0;
	bob = (thrd_t *) thrd_current();
	int i = 0;
	while (i++ < 10) {
		P1OUT ^= BIT0;
		thrd_sleep(250);
	}

	return -1;
}
int blinker(void *arg) {
	int i = 0;
	while (++i < 30) {
		P4OUT ^= BIT7;
		thrd_sleep(50);
	}

//	os_exit();
}

int bar(void *arg) {
	P4DIR |= BIT7;
	int status;
	thrd_join((thrd_t *) bob, &status);
	int i = 0;
	while (i++ < 20) {
		P4OUT ^= BIT7;
		thrd_sleep(100);
	}

	thrd_create(blinker, NULL, 1);

	return 0;
}

int printer1(void *arg) {
	uart_init();
	while (1) {
		sem_wait(&uart_sem);
//		mtx_lock(&uart_mtx);
		uart_printf("hello1\r\n");
//		thrd_sleep(20);
//		mtx_unlock(&uart_mtx);
		sem_post(&uart_sem);
	}
}

int printer2(void *arg) {
	uart_init();
	volatile int status;
	while (1) {
		status = sem_timedwait(&uart_sem, 26);
//		status = mtx_timedlock(&uart_mtx,25);
		if (status == 0) {
			uart_printf("hello2\r\n");
//			mtx_unlock(&uart_mtx);
			sem_post(&uart_sem);
		}
		thrd_sleep(10);
	}
}

int printer3(void *arg) {
	uart_init();
	while (1) {

	}
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	os_init();
	sem_init(&uart_sem, 1);
	mtx_init(&uart_mtx);

	thrd_create(foo, NULL, 2);
	thrd_create(bar, NULL, 1);
	thrd_create(printer1, NULL, 1);
	thrd_create(printer2, NULL, 1);
//	thrd_create(printer3, NULL);

	for (;;) {
	os_launch(); // -- need to fix rebooting system

	P1OUT &= ~BIT0;
	P4OUT &= ~BIT7;

	__delay_cycles(30000);
//	_low_power_mode_3();
	}
	return 0;
}

/**
 * Interrupt routine for receiving a character over UART
 **/

#pragma vector = USCI_A1_VECTOR
__attribute__((interrupt)) void USCI_A1_ISR(void) {
	switch (__even_in_range(UCA1IV, 4)) {
		case 0: { // Vector 0 - no interrupt
			break;
		}

		case 2: { // Vector 2 - RXIFG
			char recv = UCA1RXBUF;

			P1OUT |= BIT0;
			uart_send_byte(recv);
			P1OUT &= ~BIT0;

			break;
		}

		case 4: { // Vector 4 - TXIFG
			break;
		}

		default: {
			break;
		}
	}
}

/**
 * Initializes the UART for 115200 baud with a RX interrupt
 **/

void uart_init(void) {
	P4SEL |= BIT4 + BIT5;                     // P3.3,4 = USCI_A0 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // Put state machine in reset
	UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	UCA1BR1 = 0;                              // 1MHz 115200

	UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
	UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

	P4DIR |= BIT7;
	P4OUT &= ~BIT7;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

}

/**
 * uart_puts() is used by printf() to display or send a string
 **/

void uart_puts(char *s) {
	char *p;
	for (p = s; *p != 0; p++) uart_putc(*p);
}

/**
 * uart_puts() is used by printf() to display or send a character
 **/

void uart_putc(unsigned b) {
	while (UCA1STAT & UCBUSY);
	uart_send_byte(b);
}

/**
 * Sends a single byte out through UART
 **/

void uart_send_byte(unsigned char byte) {
	UCA1TXBUF = byte;
}

static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
		1000000000,// +0
		100000000, // +1
		10000000, // +2
		1000000, // +3
		100000, // +4
//       65535      // 16 bit unsigned max
		10000, // +5
		1000, // +6
		100, // +7
		10, // +8
		1, // +9
		};

static void xtoa(unsigned long x, const unsigned long *dp) {
	char c;
	unsigned long d;
	if (x) {
		while (x < *dp)
			++dp;
		do {
			d = *dp++;
			c = '0';
			while (x >= d) {
				++c;
				x -= d;
			}

			uart_putc(c);
		} while (!(d & 1));
	} else
		uart_putc('0');
}

static void puth(unsigned n) {
	static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
			'9', 'A', 'B', 'C', 'D', 'E', 'F' };
	uart_putc(hex[n & 15]);
}

void uart_printf(char *format, ...) {
	char c;
	int i;
	long n;

	va_list a;
	va_start(a, format);
	while (c = *format++) {
		if (c == '%') {
			switch (c = *format++) {
				case 's': // String
					uart_puts(va_arg(a, char*));
					break;
				case 'c':// Char
					uart_putc(va_arg(a, char));
				break;
				case 'i':// 16 bit Integer
				case 'u':// 16 bit Unsigned
					i = va_arg(a, int);
					if (c == 'i' && i < 0) i = -i, uart_putc('-');
					xtoa((unsigned)i, dv + 5);
				break;
				case 'l':// 32 bit Long
				case 'n':// 32 bit uNsigned loNg
					n = va_arg(a, long);
					if (c == 'l' && n < 0) n = -n, uart_putc('-');
					xtoa((unsigned long)n, dv);
				break;
				case 'x':// 16 bit heXadecimal
					i = va_arg(a, int);
					puth(i >> 12);
					puth(i >> 8);
					puth(i >> 4);
					puth(i);
				break;
				case 0: return;
				default: goto bad_fmt;
			}
		} else
			bad_fmt: uart_putc(c);
	}
	va_end(a);
}
