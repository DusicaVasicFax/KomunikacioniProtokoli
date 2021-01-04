#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include "Thread_declarations.h"
#include "Queue.h"

#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define MESSAGE_SIZE 512

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);}

int main(int argc, char** argv) {
	if (InitializeWindowsSockets() == false)
		return 1;

	Queue* queue = NULL;
	queue = createQueue();
	//TODO initalize queue
}