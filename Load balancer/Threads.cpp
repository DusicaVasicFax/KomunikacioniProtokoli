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

#define MESSAGE_SIZE 512

DWORD WINAPI clientListeningThread(LPVOID param) {
	ReceiveParameters* parameters = (ReceiveParameters*)param;
	printf("Client listening thread initialized... waiting for client to connect\n\n");
	int iResult;
	SOCKET listenSocket = *(parameters->listenSocket);
	SOCKET acceptedSocket = INVALID_SOCKET;
	unsigned long int nonBlockingMode = 1;

	SOCKET clientSockets[MAX_CLIENTS];
	short lastIndex = 0;

	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	do {
		char* recvbuf = (char*)malloc(MESSAGE_SIZE);
		memset(recvbuf, 0, MESSAGE_SIZE);
		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set
		if (lastIndex != MAX_CLIENTS)
		{
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < lastIndex; i++)
		{
			FD_SET(clientSockets[i], &readfds);
		}

		// wait for events on set
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed in clientListeningThread with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			Sleep(1000);
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			clientSockets[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSockets[lastIndex] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				if (ioctlsocket(clientSockets[lastIndex], FIONBIO, &nonBlockingMode) != 0)
				{
					printf("ioctlsocket in clientListeningThread failed with error.");
					continue;
				}
				lastIndex++;
			}
		}
		else
		{
			// Check if new message is received from connected clients
			for (int i = 0; i < lastIndex; i++)
			{
				// Check if new message is received from client on position "i"
				if (FD_ISSET(clientSockets[i], &readfds))
				{
					iResult = recv(clientSockets[i], recvbuf, MESSAGE_SIZE, 0);

					if (iResult > 0)
					{
						DataNode* newNode = (DataNode*)malloc(sizeof(DataNode));
						newNode->value = recvbuf;
						newNode->socket = &clientSockets[i];

						if (insertInQueue(parameters->queue, newNode) == false)
							puts("Error inserting in queue");
						printf("Message received from client in clientListening thread %s\n", recvbuf);
						printf("_______________________________  \n");
					}
					else if (iResult == 0)
					{
						// connection was closed gracefully
						printf("Connection with client (%d) closed.\n", i + 1);
						closesocket(clientSockets[i]);

						// sort array and clean last place
						for (int j = i; j < lastIndex - 1; j++)
						{
							clientSockets[j] = clientSockets[j + 1];
						}
						clientSockets[lastIndex - 1] = 0;

						lastIndex--;
					}
					else
					{
						// there was an error during recv
						printf("recv failed in clientListeningThread with error: %d\n", WSAGetLastError());
						closesocket(clientSockets[i]);

						// sort array and clean last place
						for (int j = i; j < lastIndex - 1; j++)
						{
							clientSockets[j] = clientSockets[j + 1];
						}
						clientSockets[lastIndex - 1] = 0;

						lastIndex--;
					}
				}
			}
		}
	} while (1);

	closesocket(listenSocket);
	// Deinitialize WSA library
	WSACleanup();
	return 0;
}

DWORD WINAPI dispatcher(LPVOID param) {
	ReceiveParameters* parameters = (ReceiveParameters*)param;
	Queue* queue = parameters->queue; //ovde imamo poruku i client socket
	List* list = parameters->list;

	while (1) {
		if (isEmpty(queue) == true)
		{
			/*puts("Queue je prazan!");*/
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
			current->listeningPort;
			current->active = 0;
			current->id;

			WorkerRoleData data;
			data.port = current->listeningPort; //port worker role
			data.value = node->value; //poruka
#pragma endregion

			DWORD receiverThreadId;
			ReceiveThreadParams receiveParams;

			receiveParams.receiveQueue = parameters->recQueue;
			receiveParams.port = current->listeningPort;
			receiveParams.clientSocket = node->socket;

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

	SOCKET connectSocket = CreateSocketClient((char*)"127.0.0.1", atoi(parameters->port), 0);
	// variable used to store function return value
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
	//Sleep(5000);

	printf("Bytes Sent: %ld\n", iResult);
	closesocket(connectSocket);
	return 0;
}

DWORD WINAPI receiveThread(LPVOID param) {
	//TODO receiveThreadParameters should have another reference to a queue here and to the list so they can modify them selfs
	ReceiveThreadParams* parameters = (ReceiveThreadParams*)param;

	SOCKET listenSocket = CreateSocketServer(parameters->port, 0);
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char recvbuf[MESSAGE_SIZE];
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed in receiveThread with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		//WSACleanup();
		return 1;
	}

	printf("Receiving thread started\n");

	do
	{
		// Wait for clients and accept client connections.
		// Returning value is acceptedSocket used for further
		// Client<->Server communication. This version of
		// server will handle only one client.
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
		//printf("Client should have shutted down connection");
	} while (1);

	// shutdown the connection since we're done
	iResult = shutdown(listenSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed in receiveThread with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		//WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(listenSocket);
	//closesocket(acceptedSocket);
	//WSACleanup();

	return 0;
}

DWORD WINAPI response(LPVOID param)
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

			printf("%s", node->value);

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
			Sleep(5000);

			//printf("Bytes Sent: %ld\n", iResult);
			//closesocket(connectSocket);
			return 0;
		}Sleep(1000);
	}
}