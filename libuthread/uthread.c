#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

/* enum of state of the thread */
enum
{
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    EXITED
};

/* success and failure defines */
#define SUCCESS 0
#define FAILURE -1

/* struct that holds info about the thread */
struct thread
{
    uthread_t TID;        /* thread IDentifier */
    int state;            /* running, ready, blocked, etc */
    uthread_ctx_t *uctx;  /* context: stack, registers */
    int retval;          /* the return value of thread */
};

/* define global variables */
static uthread_ctx_t main_ctx;                /* the main context thread */
static uthread_t TID_counter = 0;             /* the TID counter */
static queue_t ready_threads;                 /* a queue of the available threads */
static queue_t zombie_threads;                /* a queue of zombie threads wait for collection */
static struct thread *current_thread = NULL;  /* current running thread */

void uthread_yield(void)
{
    struct thread *next_thread;
    int ret;
    /* get the next available thread */
    ret = queue_dequeue(ready_threads, (void**)&next_thread); 

    /* check if the queue of threads is empty */
    if(ret == FAILURE)
        return;

    /* save the current thread */
    current_thread->state = READY;
    queue_enqueue(ready_threads, current_thread);
    uthread_ctx_t *current_uctx = current_thread->uctx;

    /* set current thread with new thread */
    next_thread->state = RUNNING;
    current_thread = next_thread;

    /* context switch from current to next thread */
    uthread_ctx_switch(current_uctx, next_thread->uctx);   
}

uthread_t uthread_self(void)
{
    /* if initialized return current_thread's TID
     * else return 0 (main thread)
     */
    return current_thread ? current_thread->TID : 0;
}

/*
 * uthread_init - Initializes the uthread library 
 * 
 * This functin registers the execution flow of the application as main thread
 * Return: -1 in case of failure (memory allocation)
 */
int uthread_init(void)
{ 
    /* initializes the ready thread queue */
    ready_threads = queue_create();
    zombie_threads = queue_create();

    /* memory allocation failure */
    if(!ready_threads || !zombie_threads)
	return FAILURE;
    
    /* initialize the main thread */
    current_thread = (struct thread*) malloc(sizeof(struct thread));
    
    /* memory allocation failure */
    if(!current_thread)
        return FAILURE;

    /* set current running thread the main thread */
    current_thread->TID = TID_counter;
    TID_counter++;
    current_thread->state = RUNNING;
    current_thread->uctx = &main_ctx;
    return SUCCESS;
}

int uthread_create(uthread_func_t func, void *arg)
{
    int ret = SUCCESS;
    if(TID_counter == 0)                            /* first time calling this functon */
        ret = uthread_init();

    /* in case of failure */
    if(ret == FAILURE)
	return FAILURE;
    
    /* create a new context for the thread */
    void* stack = uthread_ctx_alloc_stack();        /* allocate stack space */
    if(!stack)
	return FAILURE;
    uthread_ctx_t *uctx = (uthread_ctx_t *) malloc(sizeof(uthread_ctx_t));
    if(!uctx)
	return FAILURE;

    /* initializes the context */
    ret = uthread_ctx_init(uctx, stack, func, arg);
    if(ret == FAILURE)
	return FAILURE;

    /* check TID overflow */
    if(TID_counter == USHRT_MAX)
	return FAILURE;

    /* initialize the new thread */
    struct thread *new_thread = (struct thread*) malloc(sizeof(struct thread));
    if(!new_thread)
	return FAILURE;
    new_thread->TID = TID_counter;
    TID_counter++;
    new_thread->state = READY;
    new_thread->uctx = uctx;

    /* add the thread to queue */
    queue_enqueue(ready_threads, new_thread);
    return new_thread->TID;
}

void uthread_exit(int retval)
{
    struct thread *next_thread;
    int ret;

    /* get the next available thread */
    ret = queue_dequeue(ready_threads, (void**)&next_thread);

    /* check if thread library is initialized 
     * and there is another ready thread
     */
    if(!current_thread || ret == FAILURE)
	return;

    /* set current thread as zombie and add it to zombie queue */
    current_thread->state = ZOMBIE;
    current_thread->retval = retval;
    queue_enqueue(zombie_threads, current_thread);

    /* context switch to next thread */
    uthread_ctx_t *current_uctx = current_thread->uctx;
    next_thread->state = RUNNING;
    current_thread = next_thread;
    uthread_ctx_switch(current_uctx, next_thread->uctx);
}

int uthread_join(uthread_t tid, int *retval)
{
    int ret;
    while(1)
    {
        ret = queue_length(ready_threads);
	if(ret == 0)
	    break;
        uthread_yield();
    }    
    return SUCCESS;
}

