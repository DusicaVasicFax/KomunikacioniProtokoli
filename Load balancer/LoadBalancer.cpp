#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Thread_declarations.h"
#include "Queue.h"
#include "Sockets.h"
#include "ReceiveAndSendModel.h"
#include "List.h"
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT "5059"
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define MESSAGE_SIZE 512

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}
SOCKET clientSockets[MAX_CLIENTS];
short lastIndex = 0;

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

	List* list = NULL;
	list = createList();
	if (queue == NULL || list == NULL || recQueue == NULL)
	{
		printf("One of the queues or list have not been intialized");
		return 1;
	}
#pragma endregion

	ReceiveParameters dispatcherParams;
	dispatcherParams.queue = queue;
	dispatcherParams.recQueue = recQueue;
	dispatcherParams.list = list;
	DWORD dispatcherThreadId;

	ResponseParameters responseParams;
	responseParams.queue = recQueue;
	DWORD responseThreadId;

	CreateThread(NULL, 0, &dispatcher, &dispatcherParams, 0, &dispatcherThreadId);
	CreateThread(NULL, 0, &response, &responseParams, 0, &responseThreadId);

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

	//ReceiveParameters listeningThreadParams;
	//listeningThreadParams.listenSocket = &listenSocketServer;
	//listeningThreadParams.queue = queue;
	//listeningThreadParams.list = list;

	//DWORD clientListeningThreadId;

	//CreateThread(NULL, 0, &clientListeningThread, &listeningThreadParams, 0, &clientListeningThreadId);
	unsigned long int nonBlockingMode = 1;
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 0;
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

						if (insertInQueue(queue, newNode) == false)
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
	deleteQueue(queue);
	deleteQueue(recQueue);
	WSACleanup();

	getchar();
}