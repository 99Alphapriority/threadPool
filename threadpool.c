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

#define WORKER_COUNT 4

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
pthread_cond_t condQ;

int taskCount = 0;
int totalWorkCount = 0;

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
	taskCount++;

	/*release the lock*/
	pthread_mutex_unlock(&mutexQ);
	pthread_cond_signal(&condQ);
	return 0;
}

// remove a task from the queue
task dequeue() 
{
	task worktodo;

	if(NULL == task_q_head)
	{
		memset(&worktodo, 0, sizeof(worktodo));
		return worktodo;
	}

	task_q_st *temp = task_q_head;
	worktodo = temp->task_todo;
	task_q_head = task_q_head->next;
	taskCount--;
	totalWorkCount++;

	free(temp);

    return worktodo;
}

// the worker thread in the thread pool
void *worker(void* args)
{
	
	while(TRUE)
	{

		/*acquire lock*/
		pthread_mutex_lock(&mutexQ);

		while(0 == taskCount)
		{
			pthread_cond_wait(&condQ, &mutexQ);
			/*check if the thread is no longer needed and there is a cancellation request*/
			pthread_testcancel();
		}
		task worktodo = dequeue();

		/*release the lock*/
		pthread_mutex_unlock(&mutexQ);
		
		// execute the task
		execute(worktodo.function, worktodo.data);

		/*value should match TOTAL_WORK value in client.c*/
		if(99999 == totalWorkCount)
			break;
	}

	while(1)
		pthread_testcancel();
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
		perror("Mutex init has failed");

	if(0 != pthread_cond_init(&condQ, NULL))
		perror("Condition variable creation has failed");

	task_q_head = (task_q_st*) malloc (sizeof(task_q_st));
	task_q_tail = task_q_head;

	for(int i = 0; i < WORKER_COUNT; i++)
	{
		if(0 != pthread_create(&workers[i],NULL,worker,NULL))
			perror("Failed to create threads");
	}
}

// shutdown the thread pool
void pool_shutdown(void)
{
	for(int i = 0; i < WORKER_COUNT; i++)
	{
		/*cancel each thread*/
		pthread_cancel(workers[i]);
	}

	for(int i = 0; i < WORKER_COUNT; i++)
    {
		if(0 != pthread_join(workers[i],NULL))
			perror("Failed to join the thread");
	}
	
	pthread_mutex_destroy(&mutexQ);
	pthread_cond_destroy(&condQ);
	return;
}
