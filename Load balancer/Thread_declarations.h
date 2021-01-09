#ifndef THREADS_H_
#define THREADS_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>

DWORD WINAPI clientListeningThread(LPVOID param);
DWORD WINAPI dispatcher(LPVOID param);
DWORD WINAPI workerRole1(LPVOID param);
DWORD WINAPI workerRole2(LPVOID param);
DWORD WINAPI workerRole3(LPVOID param);

#endif