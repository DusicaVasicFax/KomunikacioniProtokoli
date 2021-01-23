#pragma once

#ifndef THREADS_H_
#define THREADS_H_

#include <ws2tcpip.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Data.h"

#define MESSAGE_SIZE sizeof(ClientProcessedRequest)

DWORD WINAPI dispatcher(LPVOID param);
DWORD WINAPI workerRole(LPVOID param);
DWORD WINAPI receiveThread(LPVOID param);
DWORD WINAPI respondToClient(LPVOID param);
#endif