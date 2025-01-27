#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Data.h"
#include "Thread_declarations.h"
#include "Queue.h"
#include "Sockets.h"
#include "List.h"
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT "5059"
#define MAX_CLIENTS 10
#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}

int numberOfClients = MAX_CLIENTS;
DWORD WINAPI receiveMessageFromClient(LPVOID param);

int main(int argc, char** argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;

#pragma region Initialization

	/*Initalized queue and checked if it initalized correctly*/
	Queue* queue = NULL;
	Queue* recQueue = NULL;
	queue = createQueue();
	recQueue = createQueue();
	List* availableWorkers = NULL;
	availableWorkers = createList(false);
	List* takenWorkers = NULL;
	takenWorkers = createList(true);
	if (queue == NULL || availableWorkers == NULL || recQueue == NULL || takenWorkers == NULL)
	{
		printf("One of the queues or list have not been intialized");
		return 1;
	}

	DispatcherParameters* dispatcherParams = (DispatcherParameters*)malloc(sizeof(DispatcherParameters));
	dispatcherParams->queue = queue;
	dispatcherParams->recQueue = recQueue;
	dispatcherParams->availableWorkers = availableWorkers;
	dispatcherParams->takenWorkers = takenWorkers;
	dispatcherParams->done = false;

	ResponseParameters* responseParams = (ResponseParameters*)malloc(sizeof(ResponseParameters));
	responseParams->queue = recQueue;
	responseParams->done = false;
	DWORD dispatcherThreadId, responseThreadId;
	HANDLE dispatch, response;

	dispatch = CreateThread(NULL, 0, &dispatcher, dispatcherParams, 0, &dispatcherThreadId);
	response = CreateThread(NULL, 0, &respondToClient, responseParams, 0, &responseThreadId);

	SOCKET listenSocket = CreateSocketServer((char*)SERVER_PORT, 1);
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Load balancer has succesfully started\n");
	SOCKET acceptedSocket = INVALID_SOCKET;
	unsigned long int nonBlockingMode = 1;
	do
	{
		iResult = Select(listenSocket, true);
		acceptedSocket = accept(listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

		DWORD receiveID;
		HANDLE hReceive;
		ClientReceiveMessageParameters clientParameters;
		clientParameters.clientSocket = &acceptedSocket;
		clientParameters.queue = queue;
		hReceive = CreateThread(NULL, 0, &receiveMessageFromClient, &clientParameters, 0, &receiveID);

		CloseHandle(hReceive);

		printf("\nPress 'x' to exit or any other key to continue: \n");
		if (_getch() == 'x')
			break;
	} while (1);

	closesocket(listenSocket);
	dispatcherParams->done = true;
	responseParams->done = true;

	CloseHandle(dispatch);
	CloseHandle(response);

	deleteQueue(queue);
	deleteQueue(recQueue);
	deleteList(availableWorkers);
	deleteList(takenWorkers);
	free(dispatcherParams);
	free(responseParams);
	WSACleanup();
	printf("Press a key to exit...");
	getchar();
	return 0;
}

DWORD WINAPI receiveMessageFromClient(LPVOID param) {
	ClientReceiveMessageParameters* parameters = (ClientReceiveMessageParameters*)param;
	SOCKET clientSocket = *(parameters->clientSocket);
	Queue* queue = parameters->queue;
	int iResult;
	char* recvbuf;
	do
	{
		recvbuf = (char*)malloc(sizeof(int));
		iResult = Select(clientSocket, true);
		// Receive data until the client shuts down the connection
		iResult = recv(clientSocket, recvbuf, sizeof(int), 0);

		if (iResult > 0)
		{
			ClientMessageReceiveAndResponseData* newNode = (ClientMessageReceiveAndResponseData*)malloc(sizeof(ClientMessageReceiveAndResponseData));
			newNode->message = recvbuf;
			newNode->socket = &clientSocket;

			if (insertInQueue(queue, newNode) == false)
				puts("Error inserting in queue");
			printf("Message received from client in clientListening thread %d\n", *(int*)recvbuf);
			printf("_______________________________  \n");
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(clientSocket);
			break;
		}
		else
		{
			// there was an error during recv
			int ierr = WSAGetLastError();
			if (ierr == WSAEWOULDBLOCK) {  // currently no data available
				Sleep(50);  // wait and try again
				continue;
			}
			printf("recv failed in receiveMessageFromClient with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			break;
		}
		free(recvbuf);
	} while (iResult > 0);
	numberOfClients--;
	return 0;
}