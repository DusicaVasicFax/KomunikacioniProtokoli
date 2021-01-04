#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Thread_declarations.h"
#include "Sockets.h"
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

	char* recvbuf = (char*)malloc(MESSAGE_SIZE);

	iResult = Select(acceptSocket, 0);
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
		iResult = Recv(acceptSocket, recvbuf);
		if (iResult > 0)
		{
			printf("Recevied message from client, proces id=%d\n", *(int*)recvbuf);
			/*CNode* cNode = Deserialize(recvbuf);
			if (insertInQueue(parameters->queue, cNode) == false)*/
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

	free(recvbuf);

	return 0;
}