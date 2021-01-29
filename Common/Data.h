#pragma once

#ifndef DATA_H_
#define DATA_H_
#include <winsock2.h>

typedef struct _ClientProcessedRequest {
	int measurmentId;
	float measuredValue;
} ClientProcessedRequest;

typedef struct _ClientMessageReceiveAndResponseData {
	SOCKET* socket;
	char* message;
	ClientProcessedRequest* data;
}ClientMessageReceiveAndResponseData;

typedef struct _Queue {
	unsigned int head;
	unsigned int tail;
	bool isFull;
	unsigned int size;
	ClientMessageReceiveAndResponseData** entries;
	CRITICAL_SECTION criticalSection;
}Queue;

typedef struct _Node {
	char* listeningPort;
	int id;
	struct _Node* next;
} Node;

typedef struct _List {
	Node* head;
	CRITICAL_SECTION criticalSection;
} List;

typedef struct _WorkerRoleData
{
	char* requestMeasurmentId;
	char* port;
	int id;
	//Node* currentWorker;
}WorkerRoleData;

typedef struct _ReceiveThreadParams
{
	Queue* receiveQueue;
	List* availableWorkers;
	List* takenWorkers;
	Node* currentReceive;
	SOCKET* clientSocket;
}ReceiveThreadParams;

typedef struct _DispatcherParameters {
	SOCKET* listenSocket;
	Queue* queue;
	Queue* recQueue;
	List* availableWorkers;
	List* takenWorkers;
	bool done;
}DispatcherParameters;

typedef struct _ResponseParameters {
	SOCKET* clientSocket;
	Queue* queue;
	bool done;
}ResponseParameters;

typedef struct _ClientReceiveMessageParameters {
	SOCKET* clientSocket;
	Queue* queue;
	int i;
} ClientReceiveMessageParameters;

#endif