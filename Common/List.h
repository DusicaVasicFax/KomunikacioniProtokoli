#pragma once

#ifndef LIST_H_
#define LIST_H_

#include "Data.h"
#include <stdlib.h>
#include <stdio.h>

void print_list(node_t* head);
void pushToEnd(node_t* head, char* ipAddress, char* listeningPort, int active);
void pushToBeginning(node_t** head, char* ipAddress, char* listeningPort, int active) {}
void freeList(node_t* head);
//int searchList(node_t* head, int value);
#endif