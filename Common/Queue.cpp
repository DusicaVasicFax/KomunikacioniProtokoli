#include "Queue.h"

Queue* createQueue(void)
{
	unsigned int size = 4;
	Queue* queue = (Queue*)malloc(sizeof(Queue));

	if (queue != NULL)
	{
		queue->data = (DataNode**)malloc(sizeof(DataNode*) * size);
		if (queue->data != NULL)
		{
			queue->size = size;
			queue->head = 0;
			queue->tail = 0;
			queue->isFull = false;
			InitializeCriticalSection(&(queue->criticalSection));
		}
		else
		{
			free(queue);
			queue = NULL;
		}
	}

	return queue;
}

void deleteQueue(Queue* queue)
{
	if (queue != NULL)
	{
		free(queue->data);
		DeleteCriticalSection(&(queue->criticalSection));
		free(queue);
	}

	return;
}


bool isEmpty(Queue* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	if (queue->head == queue->tail)
	{
		if (queue->isFull == false)
		{
			LeaveCriticalSection(&(queue->criticalSection));
			return true;
		}
	}

	LeaveCriticalSection(&(queue->criticalSection));

	return false;
}



unsigned int getSize(Queue* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	unsigned int count;

	if (isEmpty(queue) == true)
	{
		count = 0;
	}
	else if (queue->isFull == true)
	{
		count = queue->size;
	}
	else if (queue->tail > queue->head)
	{
		count = queue->tail - queue->head;
	}
	else
	{
		count = queue->size - queue->head;

		if (queue->tail > 0)
		{
			count = count + queue->tail - 1;
		}
	}

	LeaveCriticalSection(&(queue->criticalSection));

	return count;
}