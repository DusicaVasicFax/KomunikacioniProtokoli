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

typedef struct listNode {
	char* listeningPort;
	bool active;
	int id;
	struct listNode* next;
} List;

typedef struct workerRole
{
	char* value;
	char* port;
}WorkerRoleData;

typedef struct receiveThread
{
	Queue* receiveQueue;
	char* port;
	SOCKET* clientSocket;
}ReceiveThreadParams;

typedef struct receiveParameters {
	SOCKET* listenSocket;
	Queue* queue;
	Queue* recQueue;
	List* list;
}ReceiveParameters;

typedef struct responseParameters {
	SOCKET* clientSocket;
	Queue* queue;
}ResponseParameters;

#endif 