#pragma once

#ifndef Queue_H
#define Queue_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DataNode.h"
#include <windows.h>

typedef struct queue {
	unsigned int head;
	unsigned int tail;
	bool isFull;
	unsigned int size;
	DataNode **data;
	CRITICAL_SECTION criticalSection;
}Queue;

Queue* createQueue(void);
void deleteQueue(Queue* queue);
void resizeQueue(Queue* queue);
bool isEmpty(Queue* queue);
unsigned int getSize(Queue* queue);

#endif