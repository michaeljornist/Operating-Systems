#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

// STRUCTS DEFINITIONS
typedef struct Queue_obj
{
    void *data;
    struct Queue_obj *next;
    // thrd_t owner; // Owner thread ID
} Queue_obj;

// Thread object structure for the threads queue
typedef struct Thread_obj
{
    cnd_t cv; // Condition variable for the waiting thread
    struct Thread_obj *next;
} Thread_obj;

// A queue for the whating threads for item in the main queue.
typedef struct Threads_Queue
{
    Thread_obj *front;
    Thread_obj *rear;
    size_t size;
} Threads_Queue;

// Main queue structure
typedef struct Main_Queue
{
    Queue_obj *front;
    Queue_obj *rear;
    size_t size;
    size_t visited_count;
    mtx_t mutex;
    Threads_Queue waiting_threads;
    bool active;
} Main_Queue;

// Main_Queue queue;

static Main_Queue queue;

void initQueue(void)
{
    queue.front = queue.rear = NULL;
    queue.size = 0;
    queue.visited_count = 0;
    mtx_init(&queue.mutex, mtx_plain);
    queue.waiting_threads.front = queue.waiting_threads.rear = NULL;
    queue.waiting_threads.size = 0;
    queue.active = true;
}

void destroyQueue(void)
{

    mtx_lock(&queue.mutex);
    // Free all nodes in the main queue
    while (queue.front != NULL)
    {
        Queue_obj *temp = queue.front;
        queue.front = queue.front->next;
        free(temp);
    }

    queue.rear = NULL;
    queue.size = 0;
    queue.visited_count = 0;

    // Free all waiting threads in the waiting queue
    while (queue.waiting_threads.front != NULL)
    {
        Thread_obj *temp = queue.waiting_threads.front;
        queue.waiting_threads.front = queue.waiting_threads.front->next;
        cnd_destroy(&temp->cv); // Destroy the condition variable
        free(temp);
    }
    queue.waiting_threads.rear = NULL;

    queue.waiting_threads.size = 0;

    queue.active = false;
    mtx_unlock(&queue.mutex);
    mtx_destroy(&queue.mutex);
}

void enqueue(void *item)
{

    // ############## CRITICAL SECTION ##############

    mtx_lock(&(queue.mutex)); // Lock the mutex to protect the queue

    // Create a queue object from the given item.
    Queue_obj *new_item = (Queue_obj *)malloc(sizeof(Queue_obj));
    new_item->data = item;
    new_item->next = NULL;

    if (queue.rear == NULL)
    { // The main queue is empty => init the front and rear of the queue with the item.
        queue.front = new_item;
        queue.rear = new_item;
    }
    else
    {
        queue.rear->next = new_item; // The main queue not empty
        queue.rear = new_item;
    }

    queue.size++; // Increment the size of the queue

    Thread_obj *current_waiting_thread = queue.waiting_threads.front;

    // if there are threads that waiting to get the item from the queue , we will wake the oldest thread that waits for it.
    if (current_waiting_thread != NULL)
        cnd_signal(&(current_waiting_thread->cv));
    mtx_unlock(&queue.mutex);
    // ############## END OF CRITICAL SECTION ##############
}

void *dequeue(void)
{

    // ############## CRITICAL SECTION ##############
    mtx_lock(&queue.mutex); // Lock the mutex to protect the queue

    void *item = NULL;

    if (queue.waiting_threads.front == NULL && queue.front != NULL)
    {
        // If there are no waiting threads and the queue is avilable we just return the item in the queue.
    }
    else
    {
        // Thread enters here if there are waiting threads or the queue is empty.

        // Put corrent thread to the end of the waiting thread queue.
        Thread_obj *new_waiting_thread = (Thread_obj *)calloc(1, sizeof(Thread_obj));
        cnd_init(&(new_waiting_thread->cv));
        new_waiting_thread->next = NULL;

        if (queue.waiting_threads.rear == NULL)
        {
            queue.waiting_threads.front = new_waiting_thread;
            queue.waiting_threads.rear = new_waiting_thread;
        }
        else
        {
            queue.waiting_threads.rear->next = new_waiting_thread;
            queue.waiting_threads.rear = new_waiting_thread;
        }
        queue.waiting_threads.size++;

        // for the waiting thread , wait until his turn comes OR he is the first but the queue is empty.
        while (queue.waiting_threads.front != new_waiting_thread || queue.front == NULL)
        {
            if (!queue.active)
            {                             // For situations where threads are waiting for item from the queue , but meanwhile the queue destroyed.
                mtx_unlock(&queue.mutex); // Unlock the mutex
                return NULL;
            }
            cnd_wait(&(new_waiting_thread->cv), &queue.mutex);
        }

        // this line of code reatchable iff its the current thread (now the "fifo" thread) time to take the item , and the item is available.

        // remove the fifo thread that waits for item.

        queue.waiting_threads.front = queue.waiting_threads.front->next;
        if (queue.waiting_threads.front == NULL)
        {
            queue.waiting_threads.rear = NULL;
        }
        free(new_waiting_thread);

        // size_t before = queue.waiting_threads.size;
        queue.waiting_threads.size--;
        // size_t after = queue.waiting_threads.size;

        // printf("in dequeue | dequeued thread , before : %zu , after: %zu \n",before,after);


        // If there are threads that waiting and the item is availabe. the current thread (that who alrady dequeued seccesfuly)
        // will make a chain affect by calling to the second thread to take his item
        if (queue.waiting_threads.front != NULL && queue.front != NULL)
        {
            cnd_signal(&queue.waiting_threads.front->cv);
        }
    }

    Queue_obj *item_obj = queue.front;
    queue.front = queue.front->next;
    if (queue.front == NULL)
    {
        queue.rear = NULL;
    }

    item = item_obj->data;
    free(item_obj);
    queue.size--;
    queue.visited_count++;
    mtx_unlock(&(queue.mutex)); // Unlock the mutex
    // ############## END OFCRITICAL SECTION ##############
    return item;
}

bool tryDequeue(void **item)
{

    // ############## CRITICAL SECTION ##############
    mtx_lock(&queue.mutex);

    bool result = false;
    // There are items to dequeue
    if (queue.front != NULL)
    {

        Queue_obj *item_obj = queue.front;
        queue.front = queue.front->next;
        if (queue.front == NULL)
        {
            queue.rear = NULL;
        }

        *item = item_obj->data;
        free(item_obj);
        queue.size--;
        queue.visited_count++;

        result = true;
    }
    mtx_unlock(&queue.mutex);
    // ############## END CRITICAL SECTION ##############
    return result;
}

size_t size(void)
{

    size_t ret = queue.size;

    return ret;
}

size_t visited(void)
{
    return queue.visited_count;
}

size_t waiting(void)
{

    size_t ret = queue.waiting_threads.size;

    return ret;
}
