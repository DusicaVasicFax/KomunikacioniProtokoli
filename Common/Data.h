#pragma once

#ifndef DATA_H_
#define DATA_H_
#include <winsock2.h>

typedef struct dataNode {
	SOCKET* socket;
	char* value;
}DataNode;

typedef struct queue {
	unsigned int head;
	unsigned int tail;
	bool isFull;
	unsigned int size;
	DataNode** entries;
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

typedef struct workerRole
{
	char* value;
	Node* currentWorker;
}WorkerRoleData;

typedef struct _ReceiveThreadParams
{
	Queue* receiveQueue;
	List* availableWorkers;
	List* takenWorkers;
	Node* currentReceive;
	//TODO should we only send port and id here?
	SOCKET* clientSocket;
}ReceiveThreadParams;

typedef struct _DispatcherParameters {
	SOCKET* listenSocket;
	Queue* queue;
	Queue* recQueue;
	List* availableWorkers;
	List* takenWorkers;
}DispatcherParameters;

typedef struct _ResponseParameters {
	SOCKET* clientSocket;
	Queue* queue;
}ResponseParameters;

typedef struct _ClientReceiveMessageParameters {
	SOCKET* clientSocket;
	Queue* queue;
	int i;
} ClientReceiveMessageParameters;
#endif 