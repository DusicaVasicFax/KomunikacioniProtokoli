#pragma once

#ifndef LIST_H_
#define LIST_H_
#include "Data.h"
#include <stdlib.h>
#include <stdio.h>

void print_list(Node* head);
void pushToBeginning(List* list, char* listeningPort, int id);
void deleteList(List* list);
List* createList(bool empty);
void deleteFirstNodeFromList(List* list, int id);
#endif