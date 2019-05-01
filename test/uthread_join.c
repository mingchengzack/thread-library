/*
 * Thread join test
 *
 * Tests the uthread_join functions. Three threads are created in main.
 * Thread 1 waits for thread 3, threads 2 waits for thread 1, threads 3 simply runs
 * and main thread waits for thread 2
 *
 * Output:
 * thread3
 * thread1, waiting for thread3
 * thread2, waiting for thread1
 * thread0, waiting for thread2
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
	/* thread 2 waits for thread 1 */
	uthread_join(1, &retval);
	printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
	return 2;
}

int thread1(void* arg)
{
	int retval = 0;
	/* thread 1 waits for thread 3 */
        uthread_join(3, &retval);
	printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
	return 1;
}

int main(void)
{
	int retval = 0;
	/* creates 3 thread */
	uthread_create(thread1, NULL);
	uthread_create(thread2, NULL);
	uthread_create(thread3, NULL);

	/* main thread waits for thread 2 */
	uthread_join(2, &retval);
	printf("thread%d, waiting for thread%d\n", uthread_self(), retval);
	return 0;
}
