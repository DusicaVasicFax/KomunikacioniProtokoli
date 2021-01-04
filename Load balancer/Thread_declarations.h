#ifndef THREADS_H_
#define THREADS_H_
#include <ws2tcpip.h>

DWORD WINAPI clientListeningThread(LPVOID param);
bool InitializeWindowsSockets();

char* test();
#endif