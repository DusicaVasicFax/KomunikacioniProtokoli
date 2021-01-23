#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Thread_declarations.h"
#include "Sockets.h"
#include "List.h"
#include "Queue.h"

DWORD WINAPI dispatcher(LPVOID param) {
	DispatcherParameters* parameters = (DispatcherParameters*)param;
	Queue* queue = parameters->queue; //ovde imamo poruku i client socket
	List* availableWorkers = parameters->availableWorkers;
	List* takenWorkers = parameters->takenWorkers;

	while (1) {
		if (isEmpty(queue) == true)
		{
			/*puts("Queue je prazan!");*/
			Sleep(5000);
			continue;
		}
		else {
#pragma region "Create worker thread on the first available ip address"

			if (availableWorkers == NULL) {
				printf("NO AVAILABLE WORKERS CURRENTLY, WAIT A LITTLE BIT");
				//TODO maybe sleep some here?
				continue;
			}

#pragma region "POP FROM QUEUE"

			DataNode* node = (DataNode*)malloc(sizeof(DataNode));
			node = lookHead(parameters->queue);
			removeFromQueue(queue);
#pragma endregion
			Node* current = (Node*)malloc(sizeof(Node));
			EnterCriticalSection(&(availableWorkers->criticalSection));
			current->id = availableWorkers->head->id;
			current->listeningPort = availableWorkers->head->listeningPort;
			LeaveCriticalSection(&(availableWorkers->criticalSection));
			pushToBeginning(takenWorkers, current->listeningPort, current->id);

			deleteFirstNodeFromList(availableWorkers, current->id);

			WorkerRoleData data;
			data.value = node->value; //poruka
			data.currentWorker = current;

#pragma endregion

			DWORD receiverThreadId;
			ReceiveThreadParams receiveParams;

			receiveParams.receiveQueue = parameters->recQueue;
			receiveParams.clientSocket = node->socket;
			receiveParams.availableWorkers = availableWorkers;
			receiveParams.takenWorkers = takenWorkers;
			receiveParams.currentReceive = current;

			HANDLE receive, worker;
			receive = CreateThread(NULL, 0, &receiveThread, &receiveParams, 0, &receiverThreadId);

			DWORD workerRole1Id;
			worker = CreateThread(NULL, 0, &workerRole, &data, 0, &workerRole1Id);

			CloseHandle(worker);
			CloseHandle(receive);
		}
		Sleep(1000);
	}
}

DWORD WINAPI workerRole(LPVOID param)
{
	WorkerRoleData* parameters = (WorkerRoleData*)param;

	SOCKET connectSocket = CreateSocketClient((char*)"127.0.0.1", atoi(parameters->currentWorker->listeningPort), 0);
	//TODO do we need a check if this socket is created correctly ?
	printf("Worker with id: %d started successfully", parameters->currentWorker->id);

	int iResult;

	char messageToSend[100];

	strcpy_s(messageToSend, parameters->value);
	strcat_s(messageToSend, " OK");

	iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed in workerRole with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		//WSACleanup();
		return 1;
	}
	closesocket(connectSocket);

	printf("Worker should close\n");
	return 0;
}

DWORD WINAPI receiveThread(LPVOID param) {
	//TODO receiveThreadParameters should have another reference to a queue here and to the list so they can modify them selfs
	ReceiveThreadParams* parameters = (ReceiveThreadParams*)param;

	SOCKET listenSocket = CreateSocketServer(parameters->currentReceive->listeningPort, 0);
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char* recvbuf = (char*)malloc(MESSAGE_SIZE);
	memset(recvbuf, 0, MESSAGE_SIZE);
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed in receiveThread with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		//WSACleanup();
		return 1;
	}

	printf("Receiving thread stared and waiting for workers result with id: %d\n", parameters->currentReceive->id);

	acceptedSocket = accept(listenSocket, NULL, NULL);

	if (acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed in receiveThread with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		//WSACleanup();
		return 1;
	}

	// Receive data until the client shuts down the connection
	iResult = recv(acceptedSocket, recvbuf, MESSAGE_SIZE, 0);
	if (iResult > 0)
	{
		printf("Message received from worker: %s.\n", recvbuf);
		DataNode* newNode = (DataNode*)malloc(sizeof(DataNode));
		newNode->value = recvbuf;
		newNode->socket = parameters->clientSocket;

		if (!insertInQueue(parameters->receiveQueue, newNode))
		{
			printf("Error inserting in receive queue\n");
		}
		else
			printf("Added to response queue\n");
	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client closed.\n");
		closesocket(acceptedSocket);
	}
	else
	{
		// there was an error during recv
		printf("recv failed in receiveThread with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
	}

	Node* current = (Node*)malloc(sizeof(Node));
	current = parameters->takenWorkers->head;

	closesocket(listenSocket);
	//TODO delete should be done with search just to be sure
	pushToBeginning(parameters->availableWorkers, current->listeningPort, current->id);
	deleteFirstNodeFromList(parameters->takenWorkers, current->id);
	free(parameters->currentReceive);
	printf("recive thread finished\n");
	return 0;
}

DWORD WINAPI respondToClient(LPVOID param)
{
	ResponseParameters* parameters = (ResponseParameters*)param;

	while (1) {
		if (isEmpty(parameters->queue) == true)
		{
			//puts("Queue je prazan!");
			Sleep(5000);
			continue;
		}
		else
		{
			DataNode* node = (DataNode*)malloc(sizeof(DataNode));
			node = lookHead(parameters->queue);

			removeFromQueue(parameters->queue);

			printf("Got it from the response queue... sending to client... please wait: %s\n\n", node->value);

			SOCKET connectSocket = *(node->socket);
			// variable used to store function return value
			int iResult;

			iResult = send(connectSocket, node->value, (int)strlen(node->value) + 1, 0);

			if (iResult == SOCKET_ERROR)
			{
				printf("send failed in responseThread with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				//WSACleanup();
				return 1;
			}
		}
	}
	return 0;
}