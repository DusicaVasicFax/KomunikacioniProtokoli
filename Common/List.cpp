#include "List.h"

void print_list(Node* head) {
	Node* current = head;
	while (current != NULL) {
		printf("Port: %s\n", current->listeningPort);
		printf("Id:%d\n", current->id);

		current = current->next;
	}
}

List* createList(bool empty) {
	if (empty) {
		List* list = (List*)malloc(sizeof(List));
		list->head = NULL;
		InitializeCriticalSection(&list->criticalSection);
		return list;
	}
	List* list = (List*)malloc(sizeof(List));
	InitializeCriticalSection(&list->criticalSection);
	list->head = NULL;
	pushToBeginning(list, (char*)"25506", 1);
	pushToBeginning(list, (char*)"25507", 2);
	pushToBeginning(list, (char*)"25508", 3);
	pushToBeginning(list, (char*)"25509", 4);
	pushToBeginning(list, (char*)"25510", 5);
	pushToBeginning(list, (char*)"25511", 6);
	pushToBeginning(list, (char*)"25512", 7);
	pushToBeginning(list, (char*)"25513", 8);
	pushToBeginning(list, (char*)"25514", 9);
	pushToBeginning(list, (char*)"25515", 10);

	return list;
}

void pushToBeginning(List* list, char* listeningPort, int id) {
	Node* newNode = (Node*)malloc(sizeof(Node));

	EnterCriticalSection(&list->criticalSection);
	newNode->next = list->head;
	list->head = newNode;
	newNode->listeningPort = listeningPort;
	newNode->id = id;
	LeaveCriticalSection(&list->criticalSection);
}

void deleteFirstNodeFromList(List* list, int id) {
	if (list->head == NULL) return; // list already empty
	EnterCriticalSection(&list->criticalSection);

	if (list->head->id != id) {
		printf("EDGE CASE HAPPENED");
		Node* temp = list->head;
		Node* prev = NULL;
		while (temp != NULL && temp->id != id) {
			prev = temp;
			temp = temp->next;
		}
		if (temp != NULL) {
			prev->next = temp->next;
			free(temp);
		}
	}
	else {
		Node* temp = list->head;
		list->head = temp->next;
		free(temp);
	}
	LeaveCriticalSection(&list->criticalSection);
}

void deleteList(List* list) {
	if (list != NULL) {
		while (list->head != NULL) {
			Node* temp = list->head;
			if (list->head->next == NULL) {
				list->head = NULL;
			}
			else {
				list->head = list->head->next;
			}

			free(temp);
		}
		free(list->head);
		DeleteCriticalSection(&(list->criticalSection));
		free(list);
	}
	return;
}