#include "List.h"

void print_list(node_t* head) {
	node_t* current = head;
	while (current != NULL) {
		printf("Ip address: %s\n", current->ipAddress);
		printf("Port: %s\n", current->listeningPort);
		printf("Active: %d\n", current->active);

		current = current->next;
	}
}
void pushToEnd(node_t* head, char* ipAddress, char* listeningPort, int active) {
	node_t* current = head;
	while (current->next != NULL) {
		current = current->next; //loop through the list until we get to the end
	}

	current->next = (node_t*)malloc(sizeof(node_t));
	current->ipAddress = ipAddress;
	current->listeningPort = listeningPort;
	current->active = active;
	current->next->next = NULL;
}
void pushToBeginning(node_t** head, char* ipAddress, char* listeningPort, int active) {
	node_t* newNode = (node_t*)malloc(sizeof(node_t));

	newNode->next = *head; //original head pointer
	*head = newNode; //switch the two pointers
	newNode->ipAddress = ipAddress;
	newNode->listeningPort = listeningPort;
	newNode->active = active;
}
void freeList(node_t* head) {
	while (head != NULL) {
		node_t* temp = head;
		head = head->next;
		free(temp);
	}
}
int getFirstAvailable(node_t* head) {
	node_t* current = head;
	while (current != NULL) {
		if (current->active == 0)
			return 1; //TODO return IP address and port
		current = current->next;
	}
	return -1;
}