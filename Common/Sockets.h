#pragma once

#ifndef SOCKET_FUNCTIONS_H_
#define SOCKET_FUNCTIONS_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

bool InitializeWindowsSockets();
int Recv(SOCKET s, char* recvbuffer);
int Select(SOCKET s, bool receiving);
SOCKET CreateSocketServer(char* port, unsigned long int mode);
SOCKET CreateSocketClient(char* adress, int port, unsigned long int mode);

#endif