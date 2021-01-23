#pragma once

#ifndef QUEUE_H_
#define QUEUE_H_

#include "Data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

Queue* createQueue(void);
void deleteQueue(Queue* queue);
void resizeQueue(Queue* queue);
bool isEmpty(Queue* queue);
bool insertInQueue(Queue* queue, ClientMessageReceiveAndResponseData* cNode);
ClientMessageReceiveAndResponseData* removeFromQueue(Queue* queue);
ClientMessageReceiveAndResponseData* lookHead(Queue* queue);
unsigned int getSize(Queue* queue);

#endif
