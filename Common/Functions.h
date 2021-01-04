#pragma once

#ifndef Functions_H
#define Functions_H

#include <ws2tcpip.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

int recv(SOCKET s, char* recvbuffer);
int select(SOCKET s, bool receiving);
int send(SOCKET s, char* buff, int len);
SOCKET createSocketServer(char* port, unsigned long int mode);
SOCKET createSocketClient(char* adress, int port, unsigned long int mode);

#endif