#ifndef THREADS_H_
#define THREADS_H_

#include <ws2tcpip.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
DWORD WINAPI clientListeningThread(LPVOID param);
DWORD WINAPI dispatcher(LPVOID param);
DWORD WINAPI workerRole(LPVOID param);
DWORD WINAPI receiveThread(LPVOID param);
DWORD WINAPI response(LPVOID param);

#endif