/*
 * Queue test
 *
 * Test comprehensively the queue API
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <queue.h>

enum
{
    NOT_FOUND,
    FOUND
};

/* inc_item - Callback function that increments items by a certain value
 * @data: data to be incremented
 * @arg: (Optional) EXtra argument to be passed to the callback function
 *
 * This functions is used in queue_iterate to increment the item's value
 */
static int inc_item(void *data, void *arg)
{
    int *a = (int*)data;
    int inc = *((int*)arg);

    *a += inc;

    return 0;
}

/* find_item - Callback function that finds a certain item according to its value
 * @data: data to be found
 * @arg: (Optional) EXtra argument to be passed to the callback function
 *
 * This functions is usd in queue_iterate to find the item in the queue
 * and increment it by 1
 */
static int find_item(void *data, void *arg)
{
    int *a = (int*)data;
    int match = *((int*)arg);

    if (*a == match)
        return 1; 

    return 0;
}

/*
 * enqueue_array - enqueue the whole array into the queue
 * @array: Array that contains the data to be enqueued
 * @length: Length of the array
 * @queue: Queue in which to add item
 *
 * Enqueue every item in the array to the queue
 */
void enqueue_array(int *array, int length, queue_t queue)
{
    int i, ret;
    for(i = 0; i < length; i++)
    {
	/* check if each item is enqueued correctly */
        ret = queue_enqueue(queue, array + i);
	assert(ret == 0);
    }
}

/*
 * test_match - Test if the queue matches the array
 * @array: Array that contains the data to be checked with the queue
 * @length: Length of the array
 * @queue: Queue in which to be checked
 */
void test_match(int *array, int length, queue_t queue)
{
    int i, ret, *ptr;
    for(i = 0; i < length; i++)
    {
        /* check each dequeued item is same as the item in the array */
	ret = queue_dequeue(queue, (void**)&ptr);
	assert(ret == 0);            /* check if dequeued correctly */
	assert(ptr == array + i);    /* check if the item is the same */
    }

    /* check if the queue is empty */
    ret = queue_destroy(queue);
    assert(ret == 0);
}


/*
 * test_create - Unit test of the queue_create function
 *
 * Check if the created queue is not NULL
 * Check the queue created is destroyed sucessfully
 */
void test_create(void)
{
    queue_t q;
    int ret;
    printf("Testing queue_create()...\n");
    q = queue_create();
    assert(q != NULL);        /* make sure the queue is not NULL */
    ret = queue_destroy(q);
    assert(ret == 0);         /* make sure it is destroyed sucessfully */
    printf("queue_create()...OK!\n\n");
}

/*
 * test_destroy - Unit test of the queue_destroy function 
 *
 * Check if the function returns -1 if queue is NULL
 * Check if the function returns -1 if queue is not empty
 * Check if the functions returns 0 if queue is empty and destroyed
 */
void test_destroy(void)
{
    int ret;
    int data = 3, *ptr;
    queue_t q = NULL;   
    printf("Testing queue_destroy()...\n");
    
    /* Check if the function returns -1 if queue is NULL */
    ret = queue_destroy(q);
    assert(ret == -1);

    /* Check if the function returns -1 if queue is not empty */
    q = queue_create();
    queue_enqueue(q, &data);
    ret = queue_destroy(q);
    assert(ret == -1);

    /* Check if the functions returns 0 if queue is empty and destroyed */
    queue_dequeue(q, (void**)&ptr);
    ret = queue_destroy(q);
    assert(ret == 0);
    printf("queue_destroy()...OK!\n\n");
}

/*
 * test_enqueue - Unit test of the queue_enqueue function
 *
 * Check if the function returns -1 when the queue is NULL
 * Check if the function returns -1 when the data passed is NULL
 * Check if the function returns 0 when the data is enqueued
 * and also check if the data is available and in order in the queue by dequeing
 */
void test_enqueue(void)
{
    queue_t q = NULL;
    int ret;
    int data[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    printf("Testing queue_enqueue()...\n");
    
    /* Check if the function returns -1 when the queue is NULL */
    ret = queue_enqueue(q, &data[0]);
    assert(ret == -1);

    /* Check if the function returns -1 when the data passed is NULL */
    q = queue_create();
    ret = queue_enqueue(q, NULL);
    assert(ret == -1);

    /* check if the function returns 0 and correctly queued items */
    enqueue_array(data, 8, q);

    /* check if the items are in the queue and in order */
    test_match(data, 8, q);       

    printf("queue_enqueue()...OK!\n\n");
}

/*
 * test_dequeue - Unit test of the queue_dequeue function
 *
 * Check if the function returns -1 when the queue is NULL
 * Check if the function returns -1 when the queue is empty
 * Check if the function returns -1 when the data passed is NULL
 * Check if the function returns 0 when the non-empty queue is dequeued
 * and also check if the item dequeue is the oldest item
 */
void test_dequeue(void)
{
    queue_t q = NULL;
    int ret, i;
    int data[8] = {3, 4, 5, 6, 7, 8, 9, 10}, *ptr;
    printf("Testing queue_dequeue()...\n");

    /* Check if the function returns -1 when the queue is NULL */
    ret = queue_dequeue(q, (void**)&ptr);
    assert(ret == -1);

    /* Check if the function returns -1 when the queue is empty */
    q = queue_create();
    ret = queue_dequeue(q, (void**)&ptr);
    assert(ret == -1);

    /* Check if the function returns -1 when the data passed is NULL */
    enqueue_array(data, 8, q);   
    ret = queue_dequeue(q, NULL);
    assert(ret == -1);

    /* Check if the function returns 0 when the non-empty queue is dequeued
     * also check if the item dequeued is the oldest item
     */
    ret = queue_dequeue(q, (void**)&ptr);
    assert(ret == 0);
    assert(ptr == &data[0]);

    /* destroy the queue after testing */
    for(i = 0; i < 7; i++)
	queue_dequeue(q, (void**)&ptr);
    queue_destroy(q);

    printf("queue_dequeue()...OK!\n\n");
}

/*
 * test_delete - Unit test of the queue_delete function
 *
 * Check if the function returns -1 when the queue is NULL
 * Check if the function returns -1 when the data passed is NULL
 * Check if the function returns -1 when the data is not found
 * Check if the function returns 0 when the data is found 
 * and also check if the item found is deleted from the queue
 * Check if the deleted item is the oldest one
 * case 1: queue = {4, 3, 5, 3}, after delete 3, queue = {4, 5, 3}
 * case 2: queue = {3, 3, 3, 5}
 * after delete 3 twice, queue = {3, 5}
 */
void test_delete(void)
{
    queue_t q = NULL;
    int i, ret, *ptr;
    int data[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    printf("Testing queue_delete()...\n");

    /* Check if the function returns -1 when the queue is NULL */
    ret = queue_delete(q, &data[0]);
    assert(ret == -1);

    /* Check if the function returns -1 when the data passed is NULL */
    q = queue_create();
    ret = queue_delete(q, NULL);
    assert(ret == -1);

    /* Check if the function returns -1 when the data is not found */
    /* case1: when queue is empty */
    ret = queue_delete(q, &data[0]);
    assert(ret == -1);

    /* case2: when queue is not empty */
    queue_enqueue(q, &data[0]);
    ret = queue_delete(q, &data[1]);
    assert(ret == -1);

    /* Check if the function returns 0 when the data is found
     * and also check if the item found is deleted from the queue
     */
    ret = queue_delete(q, &data[0]);
    assert(ret == 0);
     
    /* make sure the item is deleted(the queue should be empty now) */
    ret = queue_destroy(q);
    assert(ret == 0);

    /* Check if the deleted item is the oldest one */
    /* case 1: queue = {4, 3, 5, 3}, after delete 3, queue = {4, 5, 3} */
    q = queue_create();
    queue_enqueue(q, &data[1]);        /* add 4 */
    queue_enqueue(q, &data[0]);        /* add 3 */
    queue_enqueue(q, &data[2]);        /* add 5 */
    queue_enqueue(q, &data[0]);        /* add 3 */

    ret = queue_delete(q, &data[0]);   /* delete the first 3 */
    assert(ret == 0);                  /* make sure it is deleted */
    queue_dequeue(q, (void**)&ptr);    /* the first dequeued item should be 4 */
    assert(ptr == &data[1]);   
    queue_dequeue(q, (void**)&ptr);    /* the second dequeued item should be 5 */
    assert(ptr == &data[2]);
    queue_dequeue(q, (void**)&ptr);    /* the second dequeued item should be 3 */
    assert(ptr == &data[0]);
    ret = queue_destroy(q);     
    assert(ret == 0);                  /* make sure the queue is empty now */
    
    /* case 2: queue = {3, 3, 3, 5}
     * after delete 3 twice, queue = {3, 5}
     */
    q = queue_create();
    queue_enqueue(q, &data[0]);        /* add 3 */
    queue_enqueue(q, &data[0]);        /* add 3 */
    queue_enqueue(q, &data[0]);        /* add 3 */
    queue_enqueue(q, &data[2]);        /* add 5 */
    
    for(i = 0; i < 2; i++)
    {
        ret = queue_delete(q, &data[0]);   /* delete 3 */
        assert(ret == 0);                  /* make sure it is deleted */
    }
    queue_dequeue(q, (void**)&ptr);    /* the first dequeued item should be 3 */
    assert(ptr == &data[0]);
    queue_dequeue(q, (void**)&ptr);    /* the second dequeued item should be 5 */
    assert(ptr == &data[2]);
    ret = queue_destroy(q);
    assert(ret == 0);                  /* make sure the queue is empty now */

    printf("queue_delete()...OK!\n\n");
}

/*
 * test_inc - Unit test of the queue_iterate with inc_item function
 * @inc: The incremental value to the queue's item
 * @data: The test array to be added to the queue
 * @match: The array to be matched after incremented
 * @length: The length of the data array
 *
 * Test inc_item with given incremental value in queue_iterate function
 */
void test_inc(int inc, int *data, int *match, int length)
{
    queue_t q = queue_create();
    int i, ret;

    /* add the array to the queue */
    enqueue_array(data, length, q);
    
    /* use queue_iterate to increment the value to the item in the queue */
    ret = queue_iterate(q, inc_item, (void*)&inc, NULL);
    assert(ret == 0);

    /* check if the item gets incremented */
    for(i = 0; i < length; i++) {
        assert(data[i] == match[i]);
    }

    /* dequeue all the items and test if they match */
    test_match(data, length, q);
}

/*
 * test_find - Unit test of the queue_iterate with find_item function
 * @find_value: The value to be found in the queue
 * @find_mode: Indicates if the item should be found or not
 * @data: The test array to be added to the queue
 * @length: The length of the data array
 * @pos: The position of the find_value in the data array
 *
 * Test find_item with given item to find in queue_iterate function
 */
void test_find(int find_value, int find_mode, int *data, int length, int pos)
{
    queue_t q = queue_create();
    int ret, *ptr = NULL;

    /* add the array to the queue */
    enqueue_array(data, length, q);

    /* use queue_iterate to find the value in the queque */
    ret = queue_iterate(q, find_item, (void*)&find_value, (void**)&ptr);
    assert(ret == 0);
    if(find_mode == FOUND)            /* if the value is found */
    {
        assert(ptr != NULL);
	assert(*ptr == find_value);
	assert(ptr == (data + pos));
    } 
    else                              /* if teh value is not found */
    {
        assert(ptr == NULL);
    }
    
    /* dequeue all the items and test if they match */
    test_match(data, length, q);
}

/*
 * test_iterate - Unit test of the queue_iterate function
 *
 * Check if the function returns -1 when the queue is NULL
 * Check if the function returns -1 when the function is NULL
 * Check if the func is successfully called on each item on the queue
 * and if the function returns 0
 * case 1: used inc_item to increment 2 to the item in the queue
 * case 2: used inc_item to increment -2 ot the item in the queue
 * case 3: used find_item to find the item 8
 * case 4: used find_item to find the item 50
 * case 5: used find_time to find the last item (10)
 */ 
void test_iterate(void)
{
    queue_t q = NULL;
    int ret;
    int case_inc_two[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    int case_inc_neg_two[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    int case_find[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    printf("Testing queue_iterate()...\n");

    /* Check if the function returns -1 when the queue is NULL */
    ret = queue_iterate(q, inc_item, (void*)1, NULL);
    assert(ret == -1);

    /* Check if the function returns -1 when the function is NULL */
    q = queue_create();
    ret = queue_iterate(q, NULL, (void*)1, NULL);
    assert(ret == -1);
    queue_destroy(q);

    /* Check if the func is successfully called on each item on the queue
     * and if the function returns 0
     */
    /* case 1: used inc_item to increment 2 to the item in the queue */
    int inc_two_match[8] = {5, 6, 7, 8, 9, 10, 11, 12};
    test_inc(2, case_inc_two, inc_two_match, 8);

    /* case 2: used inc_item to increment -2 to the item in the queue */
    int inc_neg_two_match[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    test_inc(-2, case_inc_neg_two, inc_neg_two_match, 8);

    /* case 3: used find_item to find the item 8 */
    test_find(8, FOUND, case_find, 8, 5);

    /* case 4: used find_item to find the item 50 */
    test_find(50, NOT_FOUND, case_find, 8, 5);
    
    /* case 5: used find_time to find the last item (10) */
    test_find(10, FOUND, case_find, 8, 7);

    printf("queue_iterate()...OK!\n\n");
}

/*
 * test_length - Unit test of the queue_length function
 *
 * Check if the function returns -1 when the queue is NULL
 * Check if the function returns the correct length of the queue
 * case 1: empty queue
 * case 2: enqueue an array of length 8
 * case 3: enqueue an array of length 8, and dequeue twice
 * case 4: delete an item : 10
 * case 5: when queue is empty
 */
void test_length(void)
{
    queue_t q = NULL;
    int i, ret, *ptr;
    int data[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    printf("Testing queue_length()...\n");
    
    /* Check if the function returns -1 when the queue is NULL */
    ret = queue_length(q);
    assert(ret == -1);

    /* Check if the function returns the correct length of the queue */
    /* case 1: empty queue */
    q = queue_create();
    ret = queue_length(q);
    assert(ret == 0);

    /* case 2: enqueue an array of length 8 */
    enqueue_array(data, 8, q);        /* enqueue the whole array */
    ret = queue_length(q);
    assert(ret == 8);

    /* case 3: enqueue an array of length 8, and dequeue twice */
    queue_dequeue(q, (void**)&ptr);
    queue_dequeue(q, (void**)&ptr);
    ret = queue_length(q);
    assert(ret == 6); 
    
    /* case 4: delete an item : 10 */
    queue_delete(q, &data[7]);
    ret = queue_length(q);
    assert(ret == 5);

    /* destroy the queue after testing */
    for(i = 0; i < 5; i++)
        queue_dequeue(q, (void**)&ptr);
    
    /* case 4: when queue is empty */
    ret = queue_length(q);
    assert(ret == 0);
    queue_destroy(q);

    printf("queue_length()...OK!\n\n");
}

int main(void)
{
    /* test queue_create() */
    test_create();
    
    /* test queue_destroy() */
    test_destroy();

    /* test queue_enqueue() */
    test_enqueue();
    
    /* test queue_dequeue() */
    test_dequeue();
    
    /* test queue_delete() */
    test_delete();

    /* test queue_iterate() */
    test_iterate();

    /* test queue_length() */
    test_length();
    return 0;
}
