MS-TOS is a simple work-in-progress preemptive task scheduler for the MSP430 microcontroller, written in C11 and some assembly. The scheduler is currently only tested on the MSP430FR5994 with the small code and data models. The name is a play on *MSP430*, *RTOS*, and *MS-DOS* (though it shares nothing in common with MS-DOS).

MS-TOS schedules in a round-robin fashion, with a time quantum of 2 milliseconds.

The task scheduler is statically linked with user code, and has the following usage:

```c
void thread1(void);
void thread2(void);

main() {
    os_init();
    os_thread_create(&thread1);
    os_thread_create(&thread2);
    os_run();                       // threads begin execution here
}
```