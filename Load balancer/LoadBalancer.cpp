#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Thread_declarations.h"
#include "Queue.h"
#include "Sockets.h"
#include "ReceiveAndSendModel.h"
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define MESSAGE_SIZE 512

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}

int main(int argc, char** argv) {
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;

	/*Initalized queue and checked if it initalized correctly*/
	Queue* queue = NULL;
	queue = createQueue();

	if (queue == NULL)
		return 1;

	SOCKET listenSocketServer = CreateSocketServer((char*)"27016", 1);
	iResult = listen(listenSocketServer, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketServer);
		WSACleanup();
		return 1;
	}

	printf("Load balancer has succesfully started");

	while (1)
	{
		iResult = Select(listenSocketServer, 1);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}

		ReceiveParameters parameters;
		parameters.listenSocket = &listenSocketServer;
		parameters.queue = queue;

		DWORD clientListeningThreadId;
		DWORD dispatcherThreadId;
		CreateThread(NULL, 0, &clientListeningThread, &parameters, 0, &clientListeningThreadId);
		CreateThread(NULL, 0, &dispatcher, &parameters, 0, &dispatcherThreadId);

		Sleep(500);
	}

	closesocket(listenSocketServer);
	deleteQueue(queue);
	WSACleanup();

	getchar();
}