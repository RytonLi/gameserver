#pragma once

#include <stdint.h>

#include "Utils/Define.h"

GAME_SERVICE_NS_BEGIN

struct Packet {
    uint16_t cmd;
    uint16_t len;
    const char* data;
};

GAME_SERVICE_NS_END