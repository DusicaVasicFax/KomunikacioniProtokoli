#pragma once

#ifndef QUEUE_H_
#define QUEUE_H_

#include "DataNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct queue {
	unsigned int head;
	unsigned int tail;
	bool isFull;
	unsigned int size;
	DataNode** entries;
	CRITICAL_SECTION criticalSection;
}Queue;

Queue* createQueue(void);
void deleteQueue(Queue* queue);
void resizeQueue(Queue* queue);
bool isEmpty(Queue* queue);
bool insertInQueue(Queue* queue, DataNode* cNode);
DataNode* removeFromQueue(Queue* queue);
DataNode* lookHead(Queue* queue);
unsigned int getSize(Queue* queue);

#endif
