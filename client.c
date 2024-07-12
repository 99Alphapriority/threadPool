/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

#define TOTAL_WORK 100

void add(void *param)
{
    data *temp;
    temp = (data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
    // create some work to do
    data work[TOTAL_WORK];

	for(int i = 0; i < TOTAL_WORK; i++)
	{
		work[i].a = i+3;
		work[i].b = i+5;
	}

    // initialize the thread pool
    pool_init();

    // submit the work to the queue
	for(int i = 0; i < TOTAL_WORK; i++)
		pool_submit(&add,&work[i]);

    // may be helpful 
    //sleep(3);

    pool_shutdown();

    return 0;
}
