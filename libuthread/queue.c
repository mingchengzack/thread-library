#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define FAILURE -1
#define SUCCESS 0

/* queue node that contains the data in the queue */
struct queue_node
{
    void *data;                     /* the adress of the data */
    struct queue_node *next;        /* the next queue node */
};

/* a queue data structure */
struct queue
{
    struct queue_node *head;        /* head of the queue, the oldest queued item */
    struct queue_node *tail;        /* tail of the queue, the newest queued item */
    int length;                     /* the size of the queue */    
};

queue_t queue_create(void)
{
    /* allocate memory for the queue */
    queue_t queue = (queue_t) malloc(sizeof(struct queue));

    /* failure of allocating new memory for a queue */
    if(!queue)
        return queue;

    /* initialize queue as empty */
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
    return queue; 
}

int queue_destroy(queue_t queue)
{
    /* queue is NULL or not empty */
    if(!queue || queue->head)
	return FAILURE;
  
    /* queue is empty */ 
    free(queue);            /* free the queue struct */
    return SUCCESS;
}

int queue_enqueue(queue_t queue, void *data)
{
    /* queue is NULL or data is NULL */
    if(!queue || !data)
        return FAILURE;

    /* allocate new memory for a queue node */
    struct queue_node *new_node = (struct queue_node*) 
        malloc(sizeof(struct queue_node));
    
    /* failure of allocating new memory for a new queue node */
    if(!new_node)
        return FAILURE;
    
    /* initialize new queue node */
    new_node->data = data;                              /* copy the data */
    new_node->next = NULL;                              /* initialize new node NULL */
    if(!(queue->head))                                  /* the queue is empty */
    {    
        queue->head = new_node;
	queue->tail = new_node;
    }
    else                                                /* queue is not empty */
    {
	struct queue_node *tail = queue->tail;          /* get the current tail */
        queue->tail = new_node;                         /* set the new tail */
	tail->next = new_node;                          /* connect previous tail */
    }
    queue->length++;                                    /* increment the queue size */ 
    return SUCCESS;
}

int queue_dequeue(queue_t queue, void **data)
{
    /* queue is NULL or data is NULL or queue is empty */
    if(!queue || !data || !(queue->head))
        return FAILURE;

    /* queue has item to be deleted */
    struct queue_node *delete_item = queue->head;       /* get the current head (oldest item) */
    *data = delete_item->data;                          /* assign the deleted item's value to data */
    queue->head = delete_item->next;                    /* set the next head */
    if(!queue->head)                                    /* queue becomes empty */
        queue->tail = NULL;                             /* set tail NULL as well */
    queue->length--;                                    /* decrement the queue size */
    free(delete_item);                                  /* deallocate the memory for the deleted item */
    return SUCCESS;
}

int queue_delete(queue_t queue, void *data)
{
    /* queue is NULL or data is NULL */
    if(!queue || !data)
        return FAILURE;
    
    /* queue is not empty */
    struct queue_node *current_item = queue->head;       /* get the current head */
    struct queue_node *prev_item = NULL;
    while(current_item && current_item->data != data)    /* while not the end and not found yet */
    {
        prev_item = current_item;                        /* save previous item */
	current_item = current_item->next;               /* get the next item */
    }

    /* cant find it */
    if(!current_item)
        return FAILURE;

    if(current_item == queue->head)                      /* delete the head */
        queue->head = current_item->next;                /* set the new head */          
    else                                                 /* not delete the head */       
        prev_item->next = current_item->next;            /* connect previous item to next item */

    /* delete the tail */
    if(current_item == queue->tail)
	queue->tail = prev_item;                         /* set the new tail */
    free(current_item);                                  /* deallocate the memory for the deleted item */
    return SUCCESS;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
    /* queue is NULL or func is NULL */
    if(!queue || !func)
        return FAILURE;

    /* queue and func is not NULL */
    struct queue_node *current_item = queue->head;       /* get the current head */
    
    /* go through the queue until end or func returns 1 */
    while(current_item && (*func)(current_item->data, arg) != 1) 
        current_item = current_item->next;

    /* iteration stops prematurely */
    if(current_item)
    {
	/* data is not NULL */
	if(data)
            *data = current_item->data;
    }
    return SUCCESS;
}

int queue_length(queue_t queue)
{
    /* -1 if queue is NULL, length of the queue otherwise */
    return queue ? queue->length : -1;
}

