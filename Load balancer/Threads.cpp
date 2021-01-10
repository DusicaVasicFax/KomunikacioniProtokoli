#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
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

	SOCKET connectSocket = CreateSocketClient((char*)"127.0.0.1", atoi(parameters->port), 0);
	// variable used to store function return value
	int iResult;
	char* messageToSend = (char*)"this is a test";

	iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	//Sleep(5000);

	printf("Bytes Sent: %ld\n", iResult);
	closesocket(connectSocket);
	return 0;
}

DWORD WINAPI receiveThread(LPVOID param) {
	//TODO receiveThreadParameters should have another reference to a queue here and to the list so they can modify them selfs
	WorkerRoleData* parameters = (WorkerRoleData*)param;

	SOCKET listenSocket = CreateSocketServer(parameters->port, 0);
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");

	do
	{
		// Wait for clients and accept client connections.
		// Returning value is acceptedSocket used for further
		// Client<->Server communication. This version of
		// server will handle only one client.
		acceptedSocket = accept(listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		// Receive data until the client shuts down the connection
		iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Message received from client: %s.\n", recvbuf);
			//closesocket(acceptedSocket);
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
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(acceptedSocket);
		}
		printf("Client should have shutted down connection");
	} while (1);

	// shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		//WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(listenSocket);
	closesocket(acceptedSocket);
	//WSACleanup();

	return 0;
}