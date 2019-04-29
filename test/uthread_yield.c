/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread1
 * thread2
 * thread3
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
	uthread_join(uthread_create(thread3, NULL), &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 2;
}

int thread1(void* arg)
{
	int retval = 0;
        uthread_join(uthread_create(thread2, NULL), &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 1;
}

int main(void)
{
	int retval;
	uthread_create(thread1, NULL);
	uthread_join(1, &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 0;
}
