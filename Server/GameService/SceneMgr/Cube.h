#pragma once

#include "Entity.h"

GAME_SERVICE_NS_BEGIN

class Cube : public Entity {
public:
    Cube(ENTITY_TYPE type, int id, const Transform& trans) :
        Entity(type, id, trans) {}
    virtual ~Cube() = default;

    bool Update(uint64_t diff) override;
    bool LateUpdate() override;

    bool OnEntityEnter(Entity* entity) override;
    bool OnEntityExit(Entity* entity) override;
    bool OnEntityMove(Entity* entity) override;
};

GAME_SERVICE_NS_END