#include "Queue.h"

Queue* createQueue(void)
{
	unsigned int size = 4;
	Queue* queue = (Queue*)malloc(sizeof(Queue));

	if (queue != NULL)
	{
		queue->entries = (DataNode**)malloc(sizeof(DataNode*) * size);
		if (queue->entries != NULL)
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
		free(queue->entries);
		DeleteCriticalSection(&(queue->criticalSection));
		free(queue);
	}

	return;
}

void resizeQueue(Queue* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	DataNode** temp = (DataNode**)malloc(sizeof(DataNode*) * queue->size * 2);

	if (temp != NULL)
	{
		unsigned int i = 0;
		unsigned int h = queue->head;
		do {
			temp[i] = queue->entries[h];
			h++;
			if (h == queue->size)
			{
				h = 0;
			}

			i++;
		} while (h != queue->tail);

		free(queue->entries);
		queue->entries = temp;
		queue->head = 0;
		queue->tail = queue->size;
		queue->size = queue->size * 2;
		queue->isFull = false;
	}

	LeaveCriticalSection(&(queue->criticalSection));

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

bool insertInQueue(Queue* queue, DataNode* DataNode)
{
	EnterCriticalSection(&(queue->criticalSection));

	bool result;

	if (queue->isFull == true)
	{
		resizeQueue(queue);
		if (queue->isFull == true)
		{
			result = false;
		}
	}

	if (queue->isFull == false)
	{
		queue->entries[queue->tail] = DataNode;
		queue->tail++;

		if (queue->tail == queue->size)
		{
			queue->tail = 0;
		}

		if (queue->tail == queue->head)
		{
			queue->isFull = true;
		}
	}

	LeaveCriticalSection(&(queue->criticalSection));

	return true;
}

DataNode* removeFromQueue(Queue* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	DataNode* DataNode = NULL;

	if (isEmpty(queue) == false)
	{
		if (queue->isFull == true)
		{
			queue->isFull = false;
		}

		DataNode = queue->entries[queue->head];
		queue->head++;

		if (queue->head == queue->size)
		{
			queue->head = 0;
		}
	}

	LeaveCriticalSection(&(queue->criticalSection));

	return DataNode;
}

DataNode* lookHead(Queue* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	DataNode* DataNode = NULL;

	if (isEmpty(queue) == false)
	{
		DataNode = queue->entries[queue->head];
	}

	LeaveCriticalSection(&(queue->criticalSection));

	return DataNode;
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