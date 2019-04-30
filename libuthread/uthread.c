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
    uthread_ctx_t uctx;                       /* context */
    void *stack;                              /* the stack */
    int retval;                               /* the return value of thread */
    struct thread *joined_thread;             /* the thread (blocked)that has joined to this thread */
};

/* define global variables */
static uthread_ctx_t main_ctx;                /* the main context */
static uthread_t tid_counter = 0;             /* the TID counter */
static queue_t ready_threads = NULL;          /* a queue of the available threads */
static queue_t zombie_threads = NULL;         /* a queue of zombie threads wait for collection */
static queue_t blocked_threads = NULL;        /* a queue of blockced threads */
static struct thread *current_thread = NULL;  /* current running thread */
static struct thread threads[USHRT_MAX];      /* the container for all the threads */

void uthread_yield(void)
{
    struct thread *next_thread;
    int ret;
    
    /* disable preemption
     * make sure it doesn't yield to the next of the first ready queue first
     * if it preempts after queue dequeue or after current thread is set to next thread
     */
    preempt_disable();

    /* get the next available thread */
    ret = queue_dequeue(ready_threads, (void**)&next_thread); 

    /* check if the queue of threads is empty */
    if(ret == FAILURE)
	return;

    /* save the current thread if it is running */
    if(current_thread->state == RUNNING)
    {
        /* enqueue the thread only if it is not blocked */
        current_thread->state = READY;
        queue_enqueue(ready_threads, current_thread);
    }
    uthread_ctx_t *current_uctx = &(current_thread->uctx);

    /* set current thread with new thread */
    next_thread->state = RUNNING;
    current_thread = next_thread;

    /* re-enable preemption after yieiding to the next thread */
    preempt_enable();

    /* context switch from current to next thread */
    uthread_ctx_switch(current_uctx, &(next_thread->uctx));   
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
 * This function registers and initializes the main thread for later use
 *
 */
void uthread_init(void)
{ 
    /* initialize the main thread */
    threads[tid_counter].state = RUNNING;
    threads[tid_counter].uctx = main_ctx;
    threads[tid_counter].joined_thread = NULL;
    threads[tid_counter].tid = tid_counter;

    /* set current running thread the main thread */
    current_thread = &threads[tid_counter++];

    /* start preemption */
    preempt_start();
    
    return;
}

int uthread_create(uthread_func_t func, void *arg)
{
    /* first time calling this functon */
    if(tid_counter == 0)
	uthread_init();

    /* memory allocation error */
    if(!ready_threads || !zombie_threads || !blocked_threads)
    {
        /* initializes global queue of threads */
        ready_threads = queue_create();
        zombie_threads = queue_create();
        blocked_threads = queue_create();    
    }

    /* check TID overflow */
    if(tid_counter == USHRT_MAX)
        return FAILURE;  
    
    /* allocate memory for the stack */
    void *stack = uthread_ctx_alloc_stack();
    
    /* memory allocation error */
    if(!stack)
	return FAILURE;

    /* disable preemption 
     * make sure it doesn't get overwritten by other threads
     * if other threads also call uthread_creat()
     */
    preempt_disable();

    /* initializes the context */
    int ret = uthread_ctx_init(&(threads[tid_counter].uctx), stack, func, arg);
    if(ret == FAILURE)
	return FAILURE;

    /* initializes the next thread */
    threads[tid_counter].state = READY;
    threads[tid_counter].joined_thread = NULL;
    threads[tid_counter].tid = tid_counter;
    threads[tid_counter].stack = stack;
    
    /* add the thread to queue */
    queue_enqueue(ready_threads, &threads[tid_counter++]);
    
    /* re-enable preemption */
    preempt_enable();
    
    return (tid_counter - 1);
}

void uthread_exit(int retval)
{
    /* set return value */
    current_thread->retval = retval;

    /* disable preemption
     * make sure this thread is put into zombie state
     * if the next thread is the thread that wants to join this thread
     */
    preempt_disable();

    /* create the zombie queue if not initialized */
    if(!zombie_threads)
        zombie_threads = queue_create();

    /* set current thread as zombie */
    queue_enqueue(zombie_threads, current_thread);
    current_thread->state = ZOMBIE;

    /* unblock joined thread if it has one */
    if(current_thread->joined_thread)
    {
        current_thread->joined_thread->state = READY;
	queue_enqueue(ready_threads, current_thread->joined_thread);

	/* remove thread from blocked threads queue*/
        queue_delete(blocked_threads, current_thread->joined_thread);
    }

    /* re-enable preemption after making sure the joined thread is re-queued */
    preempt_enable();
    
    /* yield to next available thread */
    uthread_yield();
}

/* delete_thread - Free the memory space allocated for the thread struct
 *
 * @t: the thread to be freed
 */
void delete_thread(struct thread *t)
{
    uthread_ctx_destroy_stack(t->stack);   /* free the stack space */
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
    
    struct thread *thread_in_ready = NULL;
    struct thread *thread_in_blocked = NULL;
    struct thread *thread_in_zombie = NULL;
    
    /* find the thread with tid in the ready threads queue */
    queue_iterate(ready_threads, find_thread, 
        (void*)&tid, (void**)&thread_in_ready);
     
    /* find the thread with tid in the blocked threads queue */
    queue_iterate(blocked_threads, find_thread,
        (void*)&tid, (void**)&thread_in_blocked);
  
    /* found the thread in ready or blocked threads */
    if(thread_in_ready || thread_in_blocked)
    {   
	struct thread *thread_to_join = thread_in_ready ? 
            thread_in_ready : thread_in_blocked;

       /* disable preemption
        * make sure the found thread can only be joined once
        * if the next thread also wants to join this found thread
        */
        preempt_disable();

	/* the thread has already been joined */
	if(thread_to_join->joined_thread)
                return FAILURE; 
        
	/* save the blocked thread (current one) */
        thread_to_join->joined_thread = current_thread;
	current_thread->state = BLOCKED;

	/* add current thread to block thread */
	queue_enqueue(blocked_threads, current_thread);

	/* re-enable preemption after registering the joined thread */
	preempt_enable();

	/* yield to next thread (it should be blocked here until joined thread died */
	uthread_yield();
    }
    
    /* find the thread with tid in the zombie threads queue */
    queue_iterate(zombie_threads, find_thread,
        (void*)&tid, (void**)&thread_in_zombie);
    
    /* found the thread in zombie threads */
    if(thread_in_zombie)
    {
	/* the thread has already been joined */
        if(thread_in_zombie->joined_thread
	    && thread_in_zombie->joined_thread != current_thread)
            return FAILURE; 

	/* delete the item from the zombie queue */
	queue_delete(zombie_threads, thread_in_zombie);

	/* main deallocates the all the queues if empty */
        if(current_thread->tid == 0 &&
            queue_length(ready_threads) == 0 &&
	    queue_length(zombie_threads) == 0 && 
	    queue_length(blocked_threads) == 0)
        {
            queue_destroy(ready_threads);
            ready_threads = NULL;
            queue_destroy(zombie_threads);
            zombie_threads = NULL;
	    queue_destroy(blocked_threads);
            blocked_threads = NULL;
	}

	/* set return value */
	if(retval)
	    *retval = thread_in_zombie->retval;

	/* free the resources with the dead thread */
        delete_thread(thread_in_zombie);
	return SUCCESS;
    }

    /* the thread cannot be found */ 
    return FAILURE;
}

