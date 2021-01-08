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
			printf("Message received from client: %s.\n", recvbuf);
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

			printf("node %s", node->value);
#pragma endregion

#pragma region "Create worker thread on the first available ip address"

			List* current = getFirstAvailable(list);
			if (current == NULL) {
				printf("ERROR ERROR ERROR I CRASHED");
			}
			current->ipAddress;
			current->listeningPort;
			current->active = 0;
#pragma endregion

			/*
			1. Pop latest info from queue;
			2. Create a worker thread on the first available ip address (params should contain a number n randomly generated to make the tread sleep)

			3. Create a receiving thread at the same time

			*/
		}
		Sleep(1000);
	}
}

DWORD WINAPI workerRole1(LPVOID param) {
	ReceiveParameters* parameters = (ReceiveParameters*)param;
	List* list = parameters->list;

	SOCKET listenSocket = INVALID_SOCKET;
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// Buffer used for storing incoming data
	char recvbuf[512];

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     //

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, "27016", &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
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

		do
		{
			// Receive data until the client shuts down the connection
			iResult = recv(acceptedSocket, recvbuf, 512, 0);
			if (iResult > 0)
			{
				printf("Message received from client: %s.\n", recvbuf);
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
		} while (iResult > 0);

		// here is where server shutdown loguc could be placed
	} while (1);

	// shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(listenSocket);
	closesocket(acceptedSocket);
	WSACleanup();

	return 0;
}