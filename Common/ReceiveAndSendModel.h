#pragma once

#ifndef RECEIVE_AND_SEND_MODEL_H_
#define RECEIVE_AND_SEND_MODEL_H_
#include "Queue.h"
#include "List.h"

typedef struct receiveParameters {
	SOCKET* listenSocket;
	Queue* queue;
	List* list;
}ReceiveParameters;

#endif