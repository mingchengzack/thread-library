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
    ZOMBIE
};

/* success and failure defines */
#define SUCCESS 0
#define FAILURE -1

/* struct that holds info about the thread */
struct thread
{
    uthread_t tid;                            /* thread identifier */
    int state;                                /* running, ready, blocked, etc */
    uthread_ctx_t *uctx;                      /* context */
    void *stack;                              /* the stack */
    int retval;                               /* the return value of thread */
    struct thread *joined_thread;             /* the thread (blocked)that has joined to this thread */
};

/* define global variables */
static uthread_ctx_t main_ctx;                /* the main context */
static struct thread main_thread;             /* the main thread */
static uthread_t tid_counter = 0;             /* the TID counter */
static queue_t ready_threads = NULL;          /* a queue of the available threads */
static queue_t zombie_threads = NULL;         /* a queue of zombie threads wait for collection */
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
    if(current_thread->state != BLOCKED)
    {
        /* enqueue the thread only if it is not blocked */
        current_thread->state = READY;
        queue_enqueue(ready_threads, current_thread);
    }
    uthread_ctx_t *current_uctx = current_thread->uctx;
   
    /* set current thread with new thread */
    next_thread->state = RUNNING;
    current_thread = next_thread;

    /* context switch from current to next thread */
    uthread_ctx_switch(current_uctx, next_thread->uctx);   
}

uthread_t uthread_self(void)
{
    /* if initialized return current thread's TID
     * else return 0 (main thread)
     */
    return current_thread ? current_thread->tid : 0;
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
    current_thread = &main_thread;

    /* set current running thread the main thread */
    current_thread->tid = tid_counter;
    tid_counter++;
    current_thread->state = RUNNING;
    current_thread->uctx = &main_ctx;
    current_thread->joined_thread = NULL;
    
    /* start preemption */
    //preempt_start();

    return SUCCESS;
}

int uthread_create(uthread_func_t func, void *arg)
{
    int ret = SUCCESS;
    /* first time calling this functon */
    if(tid_counter == 0)
	ret = uthread_init();

    /* in case of failure */
    if(ret == FAILURE)
	return FAILURE;
    
    /* check TID overflow */
    if(tid_counter == USHRT_MAX)
        return FAILURE;  
   
    /* allocate memory for a new context and the thread */
    struct thread *new_thread = (struct thread*) malloc(sizeof(struct thread));
    new_thread->stack = uthread_ctx_alloc_stack();
    new_thread->uctx = (uthread_ctx_t *) malloc(sizeof(uthread_ctx_t));
    
    /* memory allocation error */
    if(!new_thread || !new_thread->stack || !new_thread->uctx)
	return FAILURE;

    /* initializes the next thread */
    new_thread->tid = tid_counter;
    tid_counter++;
    new_thread->state = READY;
    new_thread->joined_thread = NULL;

    /* add the thread to queue */
    queue_enqueue(ready_threads, new_thread);

    /* initializes the context */
    ret = uthread_ctx_init(new_thread->uctx, new_thread->stack, func, arg);
    if(ret == FAILURE)
	return FAILURE;

    return new_thread->tid;
}

void uthread_exit(int retval)
{
    struct thread *next_thread;
    int ret;

    /* set current thread as zombie and add it to zombie queue */
    current_thread->state = ZOMBIE;
    current_thread->retval = retval;
    queue_enqueue(zombie_threads, current_thread);

    /* unblock joined thread if it has one */
    if(current_thread->joined_thread)
    {
        current_thread->joined_thread->state = READY;
        queue_enqueue(ready_threads, current_thread->joined_thread);
    }

    /* get the next available thread */
    ret = queue_dequeue(ready_threads, (void**)&next_thread);
    
    /* check if thread library is initialized
     * and there is another ready thread
     */
    if(ret == FAILURE)
        return;

    /* context switch to next thread */
    uthread_ctx_t *current_uctx = current_thread->uctx;
    next_thread->state = RUNNING;
    current_thread = next_thread;
    uthread_ctx_switch(current_uctx, next_thread->uctx);
}

/* delete_thread - Free the memory space allocated for the thread struct
 *
 * @t: the thread to be freed
 */
void delete_thread(struct thread *t)
{
    free(t->uctx);                         /* free the context */
    uthread_ctx_destroy_stack(t->stack);   /* free the stack space */
    free(t);                               /* free the thread struct */
}

/* find_thread - Callback function that finds a thread according to its id
 * @data: thread in the queue
 * @arg: (Optional) EXtra argument to be passed to the callback function
 *
 * This functions is usd in queue_iterate to find the thread in the queue
 */
static int find_thread(void *data, void *arg)
{
    struct thread *t = (struct thread*) data;
    uthread_t match_tid = *((uthread_t*)arg);
    
    if (t->tid == match_tid)
        return 1;

    return 0;
}

int uthread_join(uthread_t tid, int *retval)
{ 
    /* main thread cannot be joined or cannot join itself */
    if(tid == 0 || tid == current_thread->tid)
	return FAILURE;
    
    struct thread *thread_to_join = NULL;
 
    /* find the thread with tid in the ready threads queue */
    queue_iterate(ready_threads, find_thread, 
        (void*)&tid, (void**)&thread_to_join);
       
    /* found the thread in ready threads */
    if(thread_to_join)
    {   
	/* the thread has already been joined */
        if(thread_to_join->joined_thread)
            return FAILURE; 

	/* save the blocked thread (current one) */
        thread_to_join->joined_thread = current_thread;
	current_thread->state = BLOCKED;

	/* yield to next thread (it should be blocked here until joined thread died */
	uthread_yield();
    }
    
    /* find the thread with tid in the zombie threads queue */
    queue_iterate(zombie_threads, find_thread,
        (void*)&tid, (void**)&thread_to_join);
    
    /* found the thread in zombie threads */
    if(thread_to_join)
    {
	/* the thread has already been joined */
        if(thread_to_join->joined_thread
	    && thread_to_join->joined_thread != current_thread)
            return FAILURE; 

	/* delete the item from the zombie queue */
	queue_delete(zombie_threads, thread_to_join);

	/* set return value */
	if(retval)
	    *retval = thread_to_join->retval;

	/* free the resources with the dead thread */
        delete_thread(thread_to_join);
	return SUCCESS;
    }

    /* the thread cannot be found */ 
    return FAILURE;
}

