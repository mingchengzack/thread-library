# Program 2 Report: Thread Library
  Authors: Ming Cheng, Jiayi Xu  
  This report has the following sections
  * Overview 
  * Queue
    * Data Structures
    * Implementation
    * Testing
  * Thread
    * Data Structures
    * Implementaion
    * Testing
  * Resources
  
# Overview
  For this project, we implemented queue API and thread API.
  For the queue API, we used a special linked list that keeps track of
  the head and tail nodes of the lists.

  For the thread API, we used the queue API to store and keep track of
  the ready, blocked and zombie queues to perform corresponding API calls. 

# Queue

## Data Structures
  For implementing the queue API, we used a linked list that keeps
  track of the head and tail nodes as well as the length of the list.
  The queue node in the linked list is a struct that contains the
  (void*) data, which is just the address we want to store, and the
  next node that connects it.

## Implementation
  **queue_create()**: For this function, we malloc the memory space
  for a queue, and initialize the queue's head, tail as NULL and the
  length as zero. We will return -1 if malloc fails. Therefore, it is
  an O(1) operation.
  
  **queue_destroy()**: First we check if the queue is NULL and if the 
  queue is empty by checking if the head of the queue is NULL. If either
  one check is true, we return -1 to indicate error. Otherwise, the queue
  is empty and we can free the memory allocated for the queue and destroy it.
  Therefore, it is an O(1) operation.
  
  **queue_enqueue()**: First we do error checking by checking if queue or 
  data is NULL. If so we return -1 to indicate error. Then we allocate 
  memory for new queue node to be inserted in the queue. We initialized the
  node with the data passed in and its next node to be NULL. Then we check
  if the queue is empty. If so we set the new node as the queue's head and 
  tail. If not, we set it the new tail and connect the previous tail to it.
  Then we basically increment the length of the queue to indicate we added 
  a new node. Because enqueue always insert the node at the end of the
  linked list (tail) which we keep track of, it is an O(1) operation.
  
  **queue_dequeue()**: First we do error checking by checking if queue or
  data is NULL and if the queue is empty by checking if its head is NULL.
  If so we return -1 to indicate error. Dequeue always delete the first node
  (head) of the list so we set the next item of the previous head as the
  head and set the data as the deleted item's data. We also check if 
  after deletion the queue becomes empty. If so we set the tail NULL as
  well. Then we basically decrement the length of the queue and deallocate
  the memory with that deleted node. Since dequeue always delete the first
  node of the queue (which is only the oldest node because we insert new
  node to the end of the list), it is an O(1) operation.
  
  **queue_delete()**: First we do error checking by checking if queue or
  data is NULL. If so we return -1 to indicate error. Then we have two
  nodes to keep track of the linked list as we go through each item,
  the previous item which initialized to beNULL, and current item which
  initialized to be the head of the list. As we go over the linked list,
  we update them respectively until the end of the list or we encounter 
  the node we try to find by checking the node's data is the same as the
  data we passed from the function. Then we check if current item is NULL.
  If so, it means we could not find the item. If not, it means we found the
  item. Then we check if the deleted item is head. If so, we need to set 
  a new head which is the next item of the current item. If not, we connect
  the previous item to the next item of the current item (since current item
  will be deleted). Then we also check if we are deleting the tail (head and
  tail can be the same if the queue only has one item). If so, we set 
  previous item the new tail of the list. Then we decrement the length of
  the list and free the memory allocated for that found node.
  
  **queue_iterate()**: First we do error checking by checking queue or 
  function is NULL. If so we return -1 to indicate error. Then we basically
  go through linked list and update current item as we go along until the
  end or the function with the current item's data as the first argument
  and arg as the second argument returns 1. Then we check if current item
  is NULL. If so we go through all the linekd list. If not, we stop
  prematurely, and at this case, we also assign data with the current item's
  data.
  
  **queue_length()**: we basically return the queue's length since we keep
  track of it in each API.
  
## Testing
  For testing the queue APIs, we used unit testing to test each function 
  of the API in test_queue.c. We think of test cases that can
  trigger every edge case in our code. All the test cases are in the 
  comments of the tester.
  
# Thread

## Data Structures
  We have a struct that act as the TCB, we call it thread. It contains
  tid, state, the context, the stack pointer, return value, its joined
  thread. Since main context doesn't need to be initialized, we declare
  the main context as a global variable. We also use global variables:
  the tid counter to keep track how many threads we created, a queue of
  ready threads to keep track the next ready threads for yielding, a 
  queue of blocked threads, a queue of zombie threads that has exited
  but not connected, the current thread that is currently running (not
  included in the queue of ready threads), and the array of threads 
  we created so far.
  
## Implementation
  **uthread_yield()**: We dequeue one item from the queue of ready
  threads to get next thread we want to run. Then we check if the
  current thread is running (it could be blocked or zombie). If so we 
  also enqueue the current thread to the end of the ready queue for
  later schedule. Then we set the next thread as the current thread (
  the global variable) and used uthread_ctx_switch and stored context
  for both threads to switch context.
  
  **uthread_self()**: It returns the current thread's tid.
  
  **uthread_create()**: For the first time calling this function, we
  also call a function we create: uthread_init() to initialize the main
  thread. In that function, we initializes the main thread in our 
  global array of threads (the 0 pos) by setting its context (we
  already declare as global before) and set the current thread as 
  the pointer to that main thread. Then if not initialized
  before, we initialized all the global queues with queue_create(). 
  Then inside this function, we allocate the stack space, and 
  initialize the context with uthread_ctx_init, and add it in our 
  global array of threads and add its pointer to ready queue.
  
  **uthread_exit()**: We add the current thread to the zombie queue,
  and unblock its joined thread (if any) by enqueuing them to the
  ready queue and deleting them from the blocked queue. Then we
  yield to next thread.
  
  **uthread_join()**: First we check if we are joining main or joining
  itself. If so we return -1 to indicate error. Then we used 
  queue_iterate and find_thread with tid to find the thread in ready
  queue or blocked queue (since we also can join a blocked queue). If
  found any, we set that found thread's joined thread as the current 
  thread (for future unblocking ) and current thread as blocked and 
  add current thread to the blocked queue. Then we simply yield to
  next thread. Either we come back from at that position or we didn't
  find any in ready or blocked queue, we try to find it again in the 
  zombie queue. If found, we collect it and free its space and set
  the return value.
  
  **preempt_start()**: First we register the signal handler which is 
  simply calling uthread_yield() with the signal SIGVTALRM using signal.
  Then we set up the timer for the alarm using settimer with a frequency
  of every 0.01s.
  
  **preempt_disable()**: We use sigprocmask to tell it to block SIGVTALRM.
  
  **preempt_enable()**: We use sigprocmask to tell it to unblock SIGVTALRM.
  
## Testing
  For testing phase 2, we used the two testers provided.
  
  For testing phase 3, we wrote two testers called test_join_1.c and
  test_join_2.c to test the function of uthread_join(). We called 
  multiple join between parents and children. (the specific test cases 
  are in the comment of those testers."
  
  For testing phase 4, we wrote a tester called test_preempt.c to
  test preemption. It is a simple test. There are two infinite running
  threads, thread 1 and thread 2, and one that normally ends, thread 3. 
  After creating those threads in main, we called yield(), which
  should schedule to thread 1, which would be non-stop running if there
  is no preemption. With preemption, it shoud yield to thread 2 after
  some time, and because thread 2 also is non-stop running, it will also
  yield to thread 3 after some time. Then after thread 3 quickly ends, 
  it will yield back to main and the main program will exit to demonstrate 
  the scheduler of the preemption.

# Resources
  https://www.gnu.org/software/libc/manual/html_mono/libc.html
