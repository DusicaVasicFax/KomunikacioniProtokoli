#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Sockets.h"
#include <Serializer.h>
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5059

int __cdecl main(int argc, char** argv)
{
	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// message to send

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	printf("Enter measurment id: ");
	int id;
	scanf_s("%d", &id);
	char* messageToSend = (char*)malloc(sizeof(int));
	memcpy(messageToSend, &id, sizeof(int));

	iResult = send(connectSocket, messageToSend, sizeof(int), 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	printf("Message sent successfully\n");
	/*-------------------------------RECEIVE-------------------------*/

	char recvbuf[DEFAULT_BUFLEN];
	iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		ClientProcessedRequest* processedData = Deserialize(recvbuf);
		printf("Read measurment id: %d\n", processedData->measurmentId);
		printf("Read measurment value: %.2f", processedData->measuredValue);
		//printf("Message received from client: %s.\n", recvbuf);
	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client closed.\n");
		closesocket(connectSocket);
	}
	else
	{
		// there was an error during recv
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
	}

	printf("Enter measurment id: ");
	//int id;
	scanf_s("%d", &id);
	messageToSend = (char*)malloc(sizeof(int));
	memcpy(messageToSend, &id, sizeof(int));

	iResult = send(connectSocket, messageToSend, sizeof(int), 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	printf("Message sent successfully\n");
	/*-------------------------------RECEIVE-------------------------*/

	//char recvbuf[DEFAULT_BUFLEN];
	iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		ClientProcessedRequest* processedData = Deserialize(recvbuf);
		printf("Read measurment id: %d\n", processedData->measurmentId);
		printf("Read measurment value: %.2f", processedData->measuredValue);
		//printf("Message received from client: %s.\n", recvbuf);
	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client closed.\n");
		closesocket(connectSocket);
	}
	else
	{
		// there was an error during recv
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
	}

	/*Sleep(5000);*/
	_getch();
	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}