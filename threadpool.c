/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include "threadpool.h"
#include <unistd.h>

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3
#define WORKER_COUNT 5

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

typedef struct task_queue
{
	task task_todo;
	struct task_queue *next;
}task_q_st;

task_q_st *task_q_head = NULL;
task_q_st *task_q_tail = NULL;

pthread_t workers[5];
pthread_mutex_t mutexQ;

// the work queue
task worktodo;

// the worker bee
pthread_t bee;

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
	/*acquire the lock before adding a task to the queue*/
	pthread_mutex_lock(&mutexQ);

	task_q_tail->task_todo = t;
	task_q_tail->next = (task_q_st*) malloc (sizeof(task_q_st));

	if(NULL == task_q_tail->next)
		return 1;

	task_q_tail = task_q_tail->next;

	/*release the lock*/
	pthread_mutex_unlock(&mutexQ);

	return 0;
}

// remove a task from the queue
task dequeue() 
{
	task worktodo;
	/*acquire the lock*/
	pthread_mutex_lock(&mutexQ);

	if(NULL == task_q_head)
	{
		memset(&worktodo, 0, sizeof(worktodo));
		pthread_mutex_unlock(&mutexQ);
		return worktodo;
	}

	task_q_st *temp = task_q_head;
	worktodo = temp->task_todo;
	task_q_head = task_q_head->next;

	/*release the lock*/
	pthread_mutex_unlock(&mutexQ);

	free(temp);

    return worktodo;
}

// the worker thread in the thread pool
void *worker()
{
	sleep(10);
	while(1)
	{
		task worktodo = dequeue();
		
		if(NULL == worktodo.function)
			continue;
		// execute the task
		execute(worktodo.function, worktodo.data);
	}
    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    worktodo.function = somefunction;
    worktodo.data = p;

	if(enqueue(worktodo))
		return 1;

	return 0;
}

// initialize the thread pool
void pool_init(void)
{
	/*create and initialize the lock*/
	if(0 != pthread_mutex_init(&mutexQ, NULL))
	{
		printf("Mutex init has failed\n");
		return;
	}

	task_q_head = (task_q_st*) malloc (sizeof(task_q_st));
	task_q_tail = task_q_head;

	for(int i = 0; i < WORKER_COUNT; i++)
	{
		pthread_create(&workers[i],NULL,worker,NULL);
	}
}

// shutdown the thread pool
void pool_shutdown(void)
{
	for(int i = 0; i < WORKER_COUNT; i++)
    {
		pthread_join(workers[i],NULL);
	}
}
