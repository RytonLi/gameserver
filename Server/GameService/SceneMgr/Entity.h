#pragma once

#include "DataStruct/PlayerData.h"
#include "AOI.h"

GAME_SERVICE_NS_BEGIN

enum ENTITY_TYPE : uint8_t {
    emEntityRole = 0,
    emEntityCube = 1,
};

class Entity {
public:
    Entity(ENTITY_TYPE type, int id, const Transform& trans) :
        m_type(type),
        m_id(id),
        m_trans(trans),
        m_speed(0),
        m_isStatusChange(false),
        m_scene(nullptr),
        m_aoi(this) {}
    virtual ~Entity() = default;

    virtual bool Update(uint64_t diff) = 0;
    virtual bool LateUpdate() = 0;

    virtual bool OnEntityEnter(Entity* entity) = 0;
    virtual bool OnEntityExit(Entity* entity) = 0;
    virtual bool OnEntityMove(Entity* entity) = 0;

protected:
    friend class Scene;
    friend class Grid;
    friend class AOI;
    friend class SceneRole;

    ENTITY_TYPE m_type;

    int m_id;

    Transform m_trans;
    int       m_speed;

    bool m_isStatusChange;

    class Scene* m_scene;

    AOI m_aoi;
};

GAME_SERVICE_NS_END