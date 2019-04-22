/*
 * Queue test
 *
 * @ Tests queue creation
 * @ Tests enqueue and dequeue
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <queue.h>

void test_create(void)
{
    queue_t q;

    q = queue_create();
    assert(q != NULL);
}

void test_queue_simple(void)
{
    queue_t q;
    int data = 3, *ptr;

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data);
}

int main(void)
{
    test_create();
    test_queue_simple();
    return 0;
}
