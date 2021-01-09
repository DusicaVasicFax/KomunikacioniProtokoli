#ifndef THREADS_H_
#define THREADS_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>

DWORD WINAPI clientListeningThread(LPVOID param);
DWORD WINAPI dispatcher(LPVOID param);
DWORD WINAPI workerRole(LPVOID param);
DWORD WINAPI receiveThread(LPVOID param);

#endif