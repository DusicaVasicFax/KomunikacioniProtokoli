#pragma once

#ifndef QUEUE_CLIENT_H_
#define QUEUE_CLIENT_H_

#include "Data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

QueueClient* createQueue(void);
void deleteQueue(QueueClient* queue);
void resizeQueue(QueueClient* queue);
bool isEmpty(QueueClient* queue);
bool insertInQueue(QueueClient* queue, DataNode* cNode);
DataNode* removeFromQueue(QueueClient* queue);
DataNode* lookHead(QueueClient* queue);
unsigned int getSize(QueueClient* queue);

#endif
