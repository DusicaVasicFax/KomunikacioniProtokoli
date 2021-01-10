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

	SOCKET listenSocketServer = CreateSocketServer((char*)SERVER_PORT, 1);
	iResult = listen(listenSocketServer, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketServer);
		WSACleanup();
		return 1;
	}

	printf("Load balancer has succesfully started\n");

	ReceiveParameters listeningThreadParams;
	listeningThreadParams.listenSocket = &listenSocketServer;
	listeningThreadParams.queue = queue;
	listeningThreadParams.list = list;

	DWORD clientListeningThreadId;

	CreateThread(NULL, 0, &clientListeningThread, &listeningThreadParams, 0, &clientListeningThreadId);

	while (1)
	{
		/*iResult = Select(listenSocketServer, 1);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed in load balancer with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}*/

		Sleep(500);
	}

	closesocket(listenSocketServer);
	deleteQueue(queue);
	WSACleanup();

	getchar();
}