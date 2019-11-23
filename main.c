#include <os.h>
#include <stdarg.h>
#include <bsem_pool.h>
#include <semaphore.h>
#include <barrier.h>

void uart_putc(unsigned);
void uart_puts(char *);
void uart_send_byte(unsigned char byte);
void uart_printf(char *format, ...);
void uart_init(void);

sem_t s;
barrier_t br;
volatile int x;

/**
 * main.c
 */

int foo(void *arg) {
	P1DIR |= BIT0;
	x=0;

	while (1) {
		P1OUT ^= BIT0;
		sem_wait(&s);
		x ++;
		sem_post(&s);
		barrier_wait(&br);
		_delay_cycles(50000);
	}
}

int bar(void *arg) {
	P4DIR |= BIT7;
	while (1) {
		P4OUT ^= BIT7;
		sem_wait(&s);
		x--;
		sem_post(&s);

		barrier_wait(&br);
		_delay_cycles(50000);
	}
}

int printer1(void *arg) {
	uart_init();
	while (1) {
		_start_critical();
		uart_printf("hello1\r\n");
		_end_critical();
		__delay_cycles(50000);
	}
}

int printer2(void *arg) {
	uart_init();
	while (1) {
		_start_critical();
		uart_printf("hello2\r\n");
		_end_critical();
		_delay_cycles(50000);
	}
}

int printer3(void *arg) {
	uart_init();
	while (1) {
		_start_critical();
		uart_printf("%i\r\n", x);
		_end_critical();
		_delay_cycles(50000);
	}
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	sem_init(&s, 1);
	barrier_init(&br, 2);

	os_init();
	os_thread_create(foo, NULL);
	os_thread_create(bar, NULL);
	os_thread_create(printer1, NULL);
	os_thread_create(printer2, NULL);
	os_thread_create(printer3, NULL);
	os_launch();
//
//	bsem_pool_t x;
//	bsem_pool_init(&x);
//
//	volatile int a2 = bsem_pool_get_active_sem(&x);
//	volatile int a3 = bsem_pool_alloc(&x);
//	volatile int a6 = bsem_pool_change_active_sem(&x, 1);
//	volatile int a4 = bsem_pool_get_active_sem(&x);
//	volatile int a5 = bsem_pool_alloc(&x);
//	volatile int a8 = bsem_pool_active_sem_get_val(&x);
//	volatile int a9 = bsem_pool_active_sem_set_val(&x, 2);
//	volatile int a10 = bsem_pool_active_sem_set_val(&x, 0);
//	volatile int a11 = bsem_pool_alloc(&x);
//	volatile int a12 = bsem_pool_alloc(&x);
//	volatile int a13 = bsem_pool_alloc(&x);
//	volatile int a15 = bsem_pool_active_sem_set_val(&x, 0);
//	volatile int a14 = bsem_pool_free(&x, 3);
//	volatile int a16 = bsem_pool_free(&x, 2);
//	volatile int a17 = bsem_pool_free(&x, 1);
//	volatile int a18 = bsem_pool_free(&x, 0);
//
//	bsem_pool_wait(&x);
//	bsem_pool_wait(&x);
//	bsem_pool_signal(&x);
//	bsem_pool_signal(&x);
//
//	while(1);
//
//	while (1);
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
