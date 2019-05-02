/*
 * Thread join test 2
 *
 * Tests the uthread_join function. 
 * Main thread joins thread 1, thread 1 joins thread 2 and thread 2 joins thread 3
 *
 * Output:
 * thread3
 * thread2, waiting for thread3
 * thread1, waiting for thread2
 * thread0, waiting for thread1
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void* arg)
{
    printf("thread%d\n", uthread_self());
    return 3;
}

int thread2(void* arg)
{
    int retval = 0;
    /* thread 2 waits for thread 3 */
    uthread_join(uthread_create(thread3, NULL), &retval);
    printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
    return 2;
}

int thread1(void* arg)
{
    int retval = 0;
    /* thread 1 waits for thread 2 */
    uthread_join(uthread_create(thread2, NULL), &retval);
    printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
    return 1;
}

int main(void)
{
    int retval = 0;
    /* main thread waits for thread 1 */
    uthread_join(uthread_create(thread1, NULL), &retval);
    printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
    return 0;
}

