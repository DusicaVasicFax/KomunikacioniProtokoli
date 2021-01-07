#include "List.h"

void print_list(List* head) {
	List* current = head;
	while (current != NULL) {
		printf("Ip address: %s\n", current->ipAddress);
		printf("Port: %s\n", current->listeningPort);
		printf("Active: %d\n", current->active);

		current = current->next;
	}
}

void pushToBeginning(List** head, char* ipAddress, char* listeningPort, int active) {
	List* newNode = (List*)malloc(sizeof(List));

	newNode->next = *head; //original head pointer
	*head = newNode; //switch the two pointers
	newNode->ipAddress = ipAddress;
	newNode->listeningPort = listeningPort;
	newNode->active = active;
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
		list->ipAddress = (char*)"testIpAddress";
		list->listeningPort = (char*)"testListeningPort";
		list->next = NULL;
		//add to beginning other ip addresses
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