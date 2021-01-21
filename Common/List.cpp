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

	pushToBeginning(list, (char*)"25506", 1);
	pushToBeginning(list, (char*)"25507", 2);
	pushToBeginning(list, (char*)"25508", 3);
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
		//Tu treba implementirati to brisanje
	}
	else {
		Node* temp = list->head;
		list->head = temp->next;
		free(temp);
	}
	LeaveCriticalSection(&list->criticalSection);
}

void deleteList(List* list) {
	while (list->head != NULL) {
		Node* temp = list->head;
		list->head = list->head->next;
		free(temp);
	}
	DeleteCriticalSection(&(list->criticalSection));
	free(list);
}