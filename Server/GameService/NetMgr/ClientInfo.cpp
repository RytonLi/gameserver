#include "ClientInfo.h"
#include <stdlib.h>

gs::ClientInfo* gs::NewClientInfo(uint64_t clientID) {
	ClientInfo* info = nullptr;

    info = (ClientInfo*)malloc(sizeof(ClientInfo));
	if (!info) {
		goto Exit0;
	}

	info->clientID = clientID;
	info->peerData.now = 0;

Exit0:
	return info;
}

void gs::DeleteClientInfo(ClientInfo* info) {
	if (info) {
		delete info;
	}
}