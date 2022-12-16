#pragma once

#include <time.h>

#include "Utils/Define.h"

GAME_SERVICE_NS_BEGIN

struct ShopItem {
    int id;
    int price;
    int personalLimitMax;
    int globalLimitMax;
    int globalLimit;
    time_t startTime;
    time_t endTime;
};


GAME_SERVICE_NS_END