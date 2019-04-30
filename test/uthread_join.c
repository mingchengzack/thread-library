/*
 * Thread join test
 *
 * Tests the uthread_join
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
	//uthread_join(uthread_create(thread3, NULL), &retval);
	uthread_join(1, &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 2;
}

int thread1(void* arg)
{
	int retval = 0;
        uthread_join(3, &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 1;
}

int main(void)
{
	int retval = 0;
	uthread_create(thread1, NULL);
	uthread_create(thread2, NULL);
	uthread_create(thread3, NULL);
	uthread_join(2, &retval);
	printf("thread%d, waiting for %d\n", uthread_self(), retval);
	return 0;
}
