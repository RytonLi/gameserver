#pragma once

#include "Utils/Define.h"
#include "DataStruct/PlayerData.h"
#include "cJSON/cJSON.h"

GAME_SERVICE_NS_BEGIN

class PlayerDataConvert {
public:
	static bool ParseFromJsonData(const char* data, int len, PlayerData* playerData);
	static cJSON* ToJsonData(const PlayerData* playerData);
};

GAME_SERVICE_NS_END