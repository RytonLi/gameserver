#pragma once

#include <unordered_map>
#include "Entity.h"

GAME_SERVICE_NS_BEGIN

class Grid {
public:
    Grid() = default;
    virtual ~Grid() = default;

    bool Init(int index);
    bool UnInit();

    bool AddEntity(Entity* entity);
    bool RemoveEntity(int entityID);

    bool BroadcastEntityEnter(Entity* entity);
    bool BroadcastEntityExit(Entity* entity);
    bool BroadcastEntityMove(Entity* entity);

private:
    friend class Scene;

    int m_index;
    std::unordered_map<int, Entity*> m_entities;

};


GAME_SERVICE_NS_END