#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Thread_declarations.h"
#include "../Common/Functions.h"
#include "../Common/Queue.h"
#include "../Common/DataNode.h"

#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define MESSAGE_SIZE 512

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}

typedef struct receiveParameters {
	SOCKET* listenSocket;
	Queue* queue;
}ReceiveParameters;

DWORD WINAPI ReceiveMessageClient(LPVOID parameter);
bool InitializeWindowsSockets();


int main(char** argv) {
	
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;
	Queue* queue = NULL;
	queue = createQueue();

	if (queue == NULL)
		return 1; 

	int i = getchar();

	SOCKET listenSocketServer = createSocketServer((char*)("2770"), 1);

	iResult = listen(listenSocketServer, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketServer);
		WSACleanup();
		return 1;
	}

	printf("Podginut LoadBalancer.\n");

	while (true)
	{
		iResult = select(listenSocketServer, 1);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}

		ReceiveParameters parameters;
		parameters.listenSocket = &listenSocketServer;
		parameters.queue = queue;

		DWORD dwThreadId;
		CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);
		Sleep(1000);
	}
	
	closesocket(listenSocketServer);
	deleteQueue(queue);
	WSACleanup();
	return 0;

}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

DWORD WINAPI ReceiveMessageClient(LPVOID parameter)
{
	ReceiveParameters* parameters = (ReceiveParameters*)parameter;

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

	char* recvbuf = (char*)malloc(MESSAGE_SIZE);

	iResult = select(acceptSocket, 0);
	if (iResult == -1)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	do
	{
		memset(recvbuf, 0, MESSAGE_SIZE);
		// Receive data until the client shuts down the connection
		iResult = recv(acceptSocket, recvbuf);
		if (iResult > 0)
		{
			printf("Recevied message from client, proces id=%d\n", *(int*)recvbuf);
			//DataNode* dataNode = Deserialize(recvbuf);
			//if (insertInQueue(parameters->queue, cNode) == false)
				//puts("Error inserting in queue");
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

	free(recvbuf);
	

	return 0;
}