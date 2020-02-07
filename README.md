MS-TOS
======

Description
-----------

MS-TOS is *MSP430*, *RTOS*, and *MS-DOS* (though it shares nothing in common with MS-DOS). What does that mean?

MS-TOS is a simple work-in-progress embedded operating system for the MSP430 microcontroller family, written in C and some MSP430 assembly. The operating system has been tested on the MSP430F5529 across all code and data models, so it should theoretically work on any MSP430 with sufficient memory.

Key Features
------------

### Summary
* Features semaphores, mutexes, barriers, FIFOs, pipes, and message boxes
* Emulates POSIX standard for C11 threads - virtually identical API & implements nearly every function
* Inter-process communication
* Features task blocking / sleeping & time-keeping
* Priority inheritance
* Round-robin scheduling w/ priority
* Features idle thread for automatic power gating

### Small 
* *Tiny* memory footprint
* Only needs 1 hardware timekeeping peripheral and access to interrupts for that peripheral

#### Minimum kernel memory usage statistics (using small code & data model)
* Consists of a single, automatically injected idle thread

| Field                   | Memory Usage (B) |
|-------------------------|------------------|
| Minimum Task Stack Size | 38               |
| Kernel Variables        | 28               |
| Kernel RAM      		  | 248              |
| Kernel Code     		  | 2242             |

#### General memory usage statistics (using small code & data model)

| Structure | Memory Usage (B)                |
|-----------|---------------------------------|
| Thread    | 22 + 2N + ```sizeof(stack)```   |
| Semaphore | 8 + 2N                          |
| Mutex     | 8 + 2N                          |
| Barrier   | 28 + 6N                         |

### Fast
* Aggressive time slice of 2 ms
* Highly optimized context switching enables < 5% processor overhead at 2 ms time slice. Conducted with a core clock of 1 MHz, typical context switch time was found to be ~100 us.

### Predictable
* Static allocation, dynamic behavior
* Predictable, deterministic memory pool allocation enables compile-time analysis & flexible application design

### Configurable 
Users may:
* Adjust thread counts
* Adjust task stack sizes
* Change the time slice
* Use a variety of kernel data structures for application design

Usage
-----
Simple usage looks like this, but many more features are available than this.

```c
mtx_t mutex;
sem_t sem;

int thread1(void *arg); 
int thread2(void *arg);
int thread3(void *arg);

int main(void) {
	os_init();

	mtx_init(&mutex);
	sem_init(&sem, val = 1);
    
	thrd_create(fn = thread1, arg = NULL, priority = 1);
	thrd_create(fn = thread1, arg = NULL, priority = 2);
	thrd_create(fn = thread1, arg = NULL, priority = 3);

	os_launch();	// threads start here
}
```