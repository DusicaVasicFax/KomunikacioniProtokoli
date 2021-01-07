#pragma once

#ifndef DATA_H_
#define DATA_H_

typedef struct dataNode {
	int processId;
	char* value;
}DataNode;

typedef struct node {
	char* ipAddress;
	char* listeningPort;
	int active;
	struct node* next;
} node_t;

#endif 