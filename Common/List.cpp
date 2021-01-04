#include <stdio.h>
#include <time.h>
#include <stdlib.h>


typedef struct node {
	int val;
	struct node* next;
} node_t;

void print_lista(node_t* head)
{
	node_t* current = head;

	while (current != NULL)
	{
		printf("%d\n", current->val);
		current = current->next;
	}
}

void search(node_t* head, int searchValue)
{
	node_t* current = head;

	while (current != NULL)
	{
		if (current->val == searchValue)
		{
			printf("Value found:%d\n", current->val);
		}
		current = current->next;
	}
}

void push(node_t* head, int val)
{
	node_t* current = head;
	while (current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	current->next = (node_t*)malloc(sizeof(node_t));
	current->next->val = val;
	current->next->next = NULL;

}

void StartAdd(node_t** head, int val)
{
	node_t* new_node;
	new_node = (node_t*)malloc(sizeof(node_t));

	new_node->val = val;
	new_node->next = *head;
	*head = new_node;
}

int main()
{
	node_t* head = NULL;
	head = (node_t*)malloc(sizeof(node_t));
	if (head == NULL) {
		return 1;
	}

	head->val = 1;
	head->next = NULL;

	int x;
	clock_t start_time = clock();

	for (int i = 0; i < 100; i++)
	{
		StartAdd(&head, rand() % 1000);

	}

	print_lista(head);


	for (int i = 0; i < 100; i++)
	{
		push(head, rand() % 1000);

	}

	print_lista(head);
}
