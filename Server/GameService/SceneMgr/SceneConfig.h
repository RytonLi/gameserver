#pragma once

#include <vector>

#include "Utils/Define.h"
#include "DataStruct/PlayerData.h"

GAME_SERVICE_NS_BEGIN

struct CubeInfo {
    int      id;
    Transform trans;
};

struct RoleInitStatus {
    Transform trans;
    int speed;
};

struct SceneConfig {
    int id;
    int length;
    int width;

    RoleInitStatus roleInitStatus;

    std::vector<CubeInfo> cubes;
};

GAME_SERVICE_NS_END