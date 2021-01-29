#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include<time.h>
#include "Thread_declarations.h"
#include "Sockets.h"
#include "List.h"
#include "Queue.h"
#include "Serializer.h"

DWORD WINAPI dispatcher(LPVOID param) {
	DispatcherParameters* parameters = (DispatcherParameters*)param;
	Queue* queue = parameters->queue; //ovde imamo poruku i client socket
	List* availableWorkers = parameters->availableWorkers;
	List* takenWorkers = parameters->takenWorkers;
	printf("\nDISPATCHER STARTED\n");
	while (!parameters->done) {
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

			ClientMessageReceiveAndResponseData* node = (ClientMessageReceiveAndResponseData*)malloc(sizeof(ClientMessageReceiveAndResponseData));
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

			WorkerRoleData* data = (WorkerRoleData*)malloc(sizeof(WorkerRoleData));
			data->requestMeasurmentId = node->message; //poruka
			data->port = current->listeningPort;
			data->id = current->id;

#pragma endregion

			DWORD receiverThreadId;
			ReceiveThreadParams* receiveParams = (ReceiveThreadParams*)malloc(sizeof(ReceiveThreadParams));

			receiveParams->receiveQueue = parameters->recQueue;
			receiveParams->clientSocket = node->socket;
			receiveParams->availableWorkers = availableWorkers;
			receiveParams->takenWorkers = takenWorkers;
			receiveParams->currentReceive = current;

			HANDLE receive, worker;
			receive = CreateThread(NULL, 0, &receiveThread, receiveParams, 0, &receiverThreadId);

			DWORD workerRole1Id;
			worker = CreateThread(NULL, 0, &workerRole, data, 0, &workerRole1Id);

			CloseHandle(worker);
			CloseHandle(receive);
			//free(receiveParams);
			//free(current);
			free(node);
		}
		Sleep(1000);
	}
	printf("\nDISPATCHER THREAD FINISHED\n");
	return 0;
}

DWORD WINAPI workerRole(LPVOID param)
{
	WorkerRoleData* parameters = (WorkerRoleData*)param;

	SOCKET connectSocket = CreateSocketClient((char*)"127.0.0.1", atoi(parameters->port), 0);
	//TODO do we need a check if this socket is created correctly ?
	printf("Worker with id: %d started successfully\n", parameters->id);

	int iResult;
	srand(time(0));
	ClientProcessedRequest* processedData = (ClientProcessedRequest*)malloc(sizeof(ClientProcessedRequest));
	processedData->measuredValue = ((float)rand() / (float)(RAND_MAX)) * 3.99 * 1000;
	processedData->measurmentId = *(int*)parameters->requestMeasurmentId;

	char* message = (char*)malloc(MESSAGE_SIZE);
	message = Serialize(processedData);
	Sleep(5000);
	iResult = send(connectSocket, message, MESSAGE_SIZE, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed in workerRole with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		//WSACleanup();
		return 1;
	}
	closesocket(connectSocket);
	free(parameters);
	free(processedData);
	free(message);
	printf("Worker should close\n");
	return 0;
}

DWORD WINAPI receiveThread(LPVOID param) {
	//TODO receiveThreadParameters should have another reference to a queue here and to the list so they can modify them selfs
	ReceiveThreadParams* parameters = (ReceiveThreadParams*)param;

	SOCKET listenSocket = CreateSocketServer(parameters->currentReceive->listeningPort, 0);
	printf("Receive with id: %d started successfully\n", parameters->currentReceive->id);
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
		ClientMessageReceiveAndResponseData* newNode = (ClientMessageReceiveAndResponseData*)malloc(sizeof(ClientMessageReceiveAndResponseData));
		newNode->data = Deserialize(recvbuf);
		newNode->socket = parameters->clientSocket;

		if (!insertInQueue(parameters->receiveQueue, newNode))
		{
			printf("Error inserting in receive queue\n");
		}
		else
			printf("Added to response queue\n");
		//free(newNode);
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

	closesocket(listenSocket);
	//TODO delete should be done with search just to be sure
	pushToBeginning(parameters->availableWorkers, parameters->currentReceive->listeningPort, parameters->currentReceive->id);
	deleteFirstNodeFromList(parameters->takenWorkers, parameters->takenWorkers->head->id);
	free(parameters->currentReceive);
	free(parameters);
	free(recvbuf);
	printf("recive thread finished\n");
	return 0;
}

DWORD WINAPI respondToClient(LPVOID param)
{
	ResponseParameters* parameters = (ResponseParameters*)param;
	printf("\nRESPOND STARTED\n");
	while (!parameters->done) {
		if (isEmpty(parameters->queue) == true)
		{
			//puts("Queue je prazan!");
			Sleep(5000);
			continue;
		}
		else
		{
			ClientMessageReceiveAndResponseData* node = (ClientMessageReceiveAndResponseData*)malloc(sizeof(ClientMessageReceiveAndResponseData));
			node = lookHead(parameters->queue);

			removeFromQueue(parameters->queue);

			printf("Got it from the response queue... sending to client... please wait\n\n");

			SOCKET connectSocket = *(node->socket);
			// variable used to store function return value
			int iResult;
			char* message = Serialize(node->data);
			iResult = send(connectSocket, message, MESSAGE_SIZE, 0);

			if (iResult == SOCKET_ERROR)
			{
				printf("send failed in responseThread with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				//WSACleanup();
				return 1;
			}
			free(message);
			free(node);
		}
	}

	printf("\nRESPOND THREAD FINISHED\n");
	return 0;
}