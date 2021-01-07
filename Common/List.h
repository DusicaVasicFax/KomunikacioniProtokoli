#pragma once

#ifndef LIST_H_
#define LIST_H_

#include "Data.h"
#include <stdlib.h>
#include <stdio.h>

void print_list(List* head);
void pushToBeginning(List** head, char* ipAddress, char* listeningPort, int active);
void freeList(List* head);
List* createList(void);
List* getFirstAvailable(List* head);
#endif