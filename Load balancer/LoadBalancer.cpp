#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Thread_declarations.h"

#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}

int main(void) {
	DWORD clientListeningThreadID;
	HANDLE hClientListeningThread;
	hClientListeningThread = CreateThread(NULL, 0, &clientListeningThread, NULL, 0, &clientListeningThreadID);

	int i = getchar();
	CloseHandle(hClientListeningThread);
}