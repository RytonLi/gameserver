#pragma once

#include <vector>

#include "Utils/Define.h"

GAME_SERVICE_NS_BEGIN

enum AOI_TYPE : uint8_t {
    emAOIIncrease = 0,
    emAOIDecrease = 1,
    emAOIUnChange = 2,
};

class AOI {
public:
    AOI(class Entity* entity) :
        m_entity(entity),
        m_centerGridIndex(-1),
        m_aoiGridIndexs() {}
    virtual ~AOI() = default;

    bool Update(int desGridIndex);

    const std::vector<int>& GetAOI();

private:
    friend class Scene;

    void _GetChangeAOIs(int desGridIndex, std::vector<std::vector<int>>& output);
    void _GetAOIByIndex(int gridIndex, std::vector<int>& output);

    class Entity* m_entity;

    int m_centerGridIndex;
    std::vector<int> m_aoiGridIndexs;
};

GAME_SERVICE_NS_END