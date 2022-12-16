#pragma once

#include <stdint.h>
#include <time.h>

#include "Utils/Define.h"

GAME_SERVICE_NS_BEGIN

struct PeerData {
	char buff[64 * 1024];
	int now;
};

struct ClientInfo {
    uint64_t clientID;
	PeerData peerData;
    uint64_t expireTime;
    bool isLogin;
};

ClientInfo* NewClientInfo(uint64_t clientID);

void DeleteClientInfo(ClientInfo* info);

GAME_SERVICE_NS_END