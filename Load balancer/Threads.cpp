#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Thread_declarations.h"
#include "Sockets.h"
#include "List.h"
#include "ReceiveAndSendModel.h"

#define MESSAGE_SIZE sizeof(DataNode)
#define DEFAULT_BUFLEN 512
DWORD WINAPI clientListeningThread(LPVOID param) {
	ReceiveParameters* parameters = (ReceiveParameters*)param;
	SOCKET acceptSocket = accept(*(parameters->listenSocket), NULL, NULL);
	if (acceptSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(*(parameters->listenSocket));
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	int iResult = ioctlsocket(acceptSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	iResult = Select(acceptSocket, 0);
	if (iResult == -1)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}
	char* recvbuf = NULL;
	do
	{
		char* recvbuf = (char*)malloc(MESSAGE_SIZE);
		memset(recvbuf, 0, MESSAGE_SIZE);
		// Receive data until the client shuts down the connection
		iResult = recv(acceptSocket, recvbuf, 512, 0);
		if (iResult > 0)
		{
			//printf("Message received from client: %s.\n", recvbuf);
			//TODO serilize and desirilazie data
			DataNode* newNode = (DataNode*)malloc(sizeof(DataNode));
			newNode->value = recvbuf;
			newNode->processId = 5;

			if (insertInQueue(parameters->queue, newNode) == false)
				puts("Error inserting in queue");
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(acceptSocket);
		}
		else
		{
			// there was an error during recv
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				iResult = 1;
				continue;
			}
			else
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptSocket);
			}
		}
	} while (iResult > 0);
	if (recvbuf != NULL)
		free(recvbuf);
	return 0;
}

DWORD WINAPI dispatcher(LPVOID param) {
	ReceiveParameters* parameters = (ReceiveParameters*)param;
	Queue* queue = parameters->queue;
	List* list = parameters->list;

	while (1) {
		if (isEmpty(queue) == true)
		{
			puts("Queue je prazan!");
			Sleep(5000);
			continue;
		}
		else {
#pragma region "POP FROM QUEUE"

			DataNode* node = (DataNode*)malloc(sizeof(DataNode));
			node = lookHead(parameters->queue);
			removeFromQueue(queue);
#pragma endregion

#pragma region "Create worker thread on the first available ip address"

			List* current = getFirstAvailable(list);
			if (current == NULL) {
				printf("ERROR ERROR ERROR I CRASHED");
			}
			current->ipAddress;
			current->listeningPort;
			current->active = 0;

			WorkerRoleData data;
			data.port = (char*)"27099";
			data.value = node->value;
#pragma endregion
			DWORD receiverThreadId;
			HANDLE receive, worker;
			receive = CreateThread(NULL, 0, &receiveThread, &data, 0, &receiverThreadId);

			DWORD workerRole1Id;
			worker = CreateThread(NULL, 0, &workerRole, &data, 0, &workerRole1Id);

			CloseHandle(worker);
			CloseHandle(receive);
			/*
			1. Pop latest info from queue;
			2. Create a worker thread on the first available ip address (params should contain a number n randomly generated to make the tread sleep)

			3. Create a receiving thread at the same time

			*/
		}
		Sleep(1000);
	}
}

DWORD WINAPI workerRole(LPVOID param)
{
	WorkerRoleData* parameters = (WorkerRoleData*)param;

	SOCKET workerSocket = CreateSocketClient((char*)"127.0.0.1", atoi(parameters->port), 1);

	//TODO change the sleep number later
	printf("Worker role succesfully started..");

	char* messageToSend = parameters->value;
	// Send an prepared message with null terminator included
	int iResult = send(workerSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(workerSocket);
		WSACleanup();
		return 1;
	}

	closesocket(workerSocket);
	WSACleanup();

	return 0;
}

DWORD WINAPI receiveThread(LPVOID param) {
	//TODO receiveThreadParameters should have another reference to a queue here and to the list so they can modify them selfs
	WorkerRoleData* parameters = (WorkerRoleData*)param;
	int iResult;
	SOCKET listenSocketServer = CreateSocketServer(parameters->port, 1);
	iResult = listen(listenSocketServer, SOMAXCONN);
	char* recvbuf = NULL;
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketServer);
		WSACleanup();
		return 1;
	}
	iResult = Select(listenSocketServer, 1);
	if (iResult == SOCKET_ERROR)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		getchar();
		return 1;
	}

	SOCKET acceptSocket = accept(listenSocketServer, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(acceptSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	iResult = Select(acceptSocket, 0);
	if (iResult == -1)
	{
		fprintf(stderr, "select failed with error AAA: %ld\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	do
	{
		char* recvbuf = (char*)malloc(MESSAGE_SIZE);
		memset(recvbuf, 0, MESSAGE_SIZE);
		// Receive data until the client shuts down the connection
		iResult = recv(acceptSocket, recvbuf, 512, 0);
		if (iResult > 0)
		{
			printf("Message received from worker: %s.\n", recvbuf);
			printf("Connection with worker closed.\n");
			closesocket(acceptSocket);
			break;
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed AAAAAAAAA.\n");
			closesocket(acceptSocket);
		}
		else
		{
			// there was an error during recv
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				iResult = 1;
				continue;
			}
			else
			{
				printf("recv failed with error AAAAAAAAAAAAA: %d\n", WSAGetLastError());
				closesocket(acceptSocket);
			}
		}
	} while (iResult > 0);
	closesocket(acceptSocket);
	if (recvbuf != NULL)
		free(recvbuf);
	return 0;
}