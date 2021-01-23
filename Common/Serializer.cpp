#include "Serializer.h"

char* Serialize(ClientProcessedRequest* request) {
	char* buffer = (char*)malloc(sizeof(ClientProcessedRequest));
	*((float*)buffer) = request->measuredValue;
	*((int*)(buffer + sizeof(float))) = request->measurmentId;
	return buffer;
}

ClientProcessedRequest* Deserialize(char* buffer) {
	ClientProcessedRequest* req = (ClientProcessedRequest*)malloc(sizeof(ClientProcessedRequest));
	req->measuredValue = *((float*)buffer);
	req->measurmentId = *((int*)(buffer + sizeof(float)));
	return req;
}