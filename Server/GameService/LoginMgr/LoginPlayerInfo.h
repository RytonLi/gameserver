#pragma once

#include <stdint.h>

#include "Utils/Define.h"

GAME_SERVICE_NS_BEGIN

struct LoginPlayerInfo {
	uint64_t clientID;
	std::string playerID;
    bool isSelectRole;
    uint64_t expireTime;

    LoginPlayerInfo(uint64_t clientID, const std::string& playerID)
		: clientID(clientID), playerID(playerID), isSelectRole(false) {}
};

GAME_SERVICE_NS_END