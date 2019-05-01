#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include <preempt.h>

/*
 * Simple preemption test
 *
 * Tests the preemption functionality. Three threads are created in main.
 * main yields to thread 1. Thread 1 should never end but it will preempt
 * to thread 2, and thread 2 also never ends but it will preempt to thread 3
 * and thread 3 will finish after printing out message, and it goes back to
 * main to the finish the program
 *
 * Output:
 * thread1 started
 * thread2 started
 * thread3 finished
 * thread0 finished
 */

int thread3(void* arg)
{
    printf("thread%d finished\n", uthread_self());
    return 3;
}

int thread2(void* arg)
{
    printf("thread2 started\n");
    /* this thread won't end */
    while(1)
    {

    }

    /* we never get here */
    printf("thread%d finished\n", uthread_self());
    return 2;
}

int thread1(void* arg)
{
    printf("thread1 started\n");
    /* this thread won't end */
    while(1)
    {

    }
    
    /* we never get here */
    printf("thread%d finished\n", uthread_self());
    return 1;
}

int main(void)
{   
    /* creates 3 threads */
    uthread_create(thread1, NULL);
    uthread_create(thread2, NULL);
    uthread_create(thread3, NULL);

    /* yield to thread 1 */
    uthread_yield();
    printf("thread%d finished\n", uthread_self());
    return 0;
}
