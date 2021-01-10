#include "List.h"

void print_list(List* head) {
	List* current = head;
	while (current != NULL) {
		printf("Port: %s\n", current->listeningPort);
		printf("Active: %d\n", current->active);
		printf("Id:%d\n", current->id);

		current = current->next;
	}
}

void pushToBeginning(List** head, char* listeningPort, bool active,int id) {
	List* newNode = (List*)malloc(sizeof(List));

	newNode->next = *head; //original head pointer
	*head = newNode; //switch the two pointers
	newNode->listeningPort = listeningPort;
	newNode->active = active;
	newNode->id = id;
}

void freeList(List* head) {
	while (head != NULL) {
		List* temp = head;
		head = head->next;
		free(temp);
	}
}

List* createList(void) {
	List* list = (List*)malloc(sizeof(List));

	if (list != NULL) {
		list->active = 0;
		list->listeningPort = (char*)"testListeningPort";
		list->next = NULL;

		pushToBeginning(&list, (char*)"25506", false,1);
		pushToBeginning(&list, (char*)"25507", false,2);
		pushToBeginning(&list, (char*)"25508", false,3);
	}
	return list;
}

List* getFirstAvailable(List* head) {
	List* current = head;
	while (current != NULL) {
		if (current->active == 0) return current;
		current = current->next;
	}
	return NULL;
}