#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

/*
 * forceful_yield - forcefully yield to next thread
 *
 * @signnum: signal whose behavior you want to control
 * 
 * The timer handler that forcefuly yields the currently running thread
 */
void forceful_yield (int signum)
{
    uthread_yield();
}

void preempt_disable(void)
{
    
}

void preempt_enable(void)
{
    
}

void preempt_start(void)
{
    struct itimerval timer;
    
    /* install the timer handler to forcefully yield */
    signal(SIGVTALRM, forceful_yield);
    
    /* set up the time elapse to every 0.01s */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;

    /* set up timer interval between two alarms */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1;

    /* set up the virtual timer for process time */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

