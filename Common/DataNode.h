#pragma once

#ifndef DATA_NODE_H_
#define DATA_NODE_H_

typedef struct dataNode {
	int processId;
	char value[512];
}DataNode;

#endif 