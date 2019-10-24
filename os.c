/*
 * os.c
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */
#include "os.h"

struct context;
struct tcb;

inline void preempt_trigger(void);
inline void preempt_init(void);
void preempt_reset(void);
extern void preempt_firstrun(void);

/* asm functions */
extern void context_save(struct context *);
extern void context_load(struct context *);

void schedule(void);
int link(void);
void panic(int) __attribute__ ((noreturn));

uint16_t num_ctx_switches = 0;

struct {
  int a;
} const scheduler = { .a = 1 };

struct context {
  word_t r4, r5, r6, r7, r8, r9, r10, sp, pc;
};

struct tcb {
  union {
    struct context ctx;
    word_t regs[9];
  };
  struct tcb *next;
  bool available;
};

struct tcb *run_ptr;
word_t stacks[NUMTHREADS][STACKSIZE];
struct tcb tcbs[NUMTHREADS];
unsigned run_ct;

void
preempt_init(void)
{
  WDTCTL = WDTPW | WDTSSEL__SMCLK | WDTTMSEL | WDTCNTCL | WDTIS__8192;
//  WDTCTL = WDT_ADLY_1_9;
  SFRIE1 |= WDTIE;
}

void
preempt_trigger(void)
{
  SFRIFG1 |= WDTIFG; // wdt interrupt pending
}

void
preempt_reset(void)
{
  SFRIFG1 &= ~WDTIFG; // no interrupt pending
  WDTCTL = WDTPW | WDTSSEL__SMCLK | WDTTMSEL | WDTCNTCL | WDTIS__8192;
  WDTCTL = WDT_ADLY_1_9;
  SFRIE1 |= WDTIE;
}

//void zero_stack(word_t *stack) {
//  int i;
//  for (i = 0; i < STACKSIZE; i++)
//    stack[i] = 0;
//}

int
link(void)
{
  int i;
  int ct = 0;
  struct tcb *prev = 0, *first = 0;
  if (NUMTHREADS == 0)
    return ct;
  for (i = 0; i < NUMTHREADS; i++) {
    if (!tcbs[i].available) {
      ct++;
      if (!first)              // get reference to first to wrap around at the end
        first = &tcbs[i];
      if (prev)                // only set previous's next if previous has been set
        prev->next = &tcbs[i]; // (accounts for first iteration of the loop)
      prev = &tcbs[i];
    }
  }
  prev->next = first;
  run_ptr = first;
  return ct;
}

thread_t
os_thread_create(void (* routine)(void))
{
  int i;
  __disable_interrupt();
  for (i = 0; i < NUMTHREADS; ++i) {
    if (tcbs[i].available) {
      tcbs[i].available = false;
      tcbs[i].ctx.sp = (word_t) &stacks[i][STACKSIZE - 1];
      tcbs[i].ctx.pc = (word_t) routine;
      (void) link();
      __enable_interrupt();
      return i;
    }
  }
  __enable_interrupt();
  return -1;
}

void
os_init(void)
{
  __disable_interrupt(); // disable interrupts until os_run
  preempt_reset();
  int i;
  for (i = 0; i < NUMTHREADS; ++i)
    tcbs[i].available = 0;
}

void
os_run(void)
{
//  if (run_ct) {
//    preempt_init();
//    context_load(&run_ptr->ctx);
//  } else {
//    for (;;);
//  }
  preempt_init();
  preempt_firstrun();
  panic(0);
}

void
os_yield(void)
{
  preempt_trigger();
}

void
os_thread_set(void (*routine1)(void),
              void (*routine2)(void))
{
  tcbs[0].available = false;
  tcbs[0].ctx.sp = (word_t) &stacks[0][STACKSIZE - 1];
  tcbs[0].ctx.pc = (word_t) routine1;

  tcbs[1].available = false;
  tcbs[1].ctx.sp = (word_t) &stacks[0][STACKSIZE - 1];
  tcbs[1].ctx.pc = (word_t) routine2;

  tcbs[0].next = &tcbs[1];
  tcbs[1].next = &tcbs[0];
  run_ptr = &tcbs[0];
}

void
schedule(void)
{
  run_ptr = run_ptr->next;
}

void
panic(c)
  int c;
{
  volatile int code = c; // for debug
  for (;;)
    ;
}

// Watchdog Timer interrupt service routine
//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=WDT_VECTOR
//__interrupt
//#elif defined(__GNUC__)
//
//__attribute__ ((interrupt(WDT_VECTOR)))
//#else
//#error Compiler not supported!
//#endif
//void
//WDT_ISR(void)
//{
//  context_save(&run_ptr->ctx);
//  schedule();
//  preempt_reset();
//  context_load(&run_ptr->ctx);
//}


