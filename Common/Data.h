#pragma once

#ifndef DATA_H_
#define DATA_H_

typedef struct dataNode {
	int processId;
	char* value;
}DataNode;

typedef struct listNode {
	char* ipAddress;
	char* listeningPort;
	int active;
	struct listNode* next;
} List;

typedef struct workerRole
{
	char* value;
	char* port;
}WorkerRoleData;

#endif 