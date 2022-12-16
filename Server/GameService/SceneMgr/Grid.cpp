#include "Grid.h"
#include <ServiceMgr.h>

using namespace std;
using namespace gs;

bool Grid::Init(int index) {
    m_index = index;
    return true;
}

bool Grid::UnInit() {
    return true;
}

bool Grid::AddEntity(Entity* entity) {
    bool ret = false;
    auto it = m_entities.find(entity->m_id);
    if (it != m_entities.end()) {
        LOG_DEBUG("grid add entity fail, entityID: %d exist", entity->m_id);
        goto Exit0;
    }

    m_entities.insert(pair<int, Entity*>(entity->m_id, entity));

    ret = true;
Exit0:
    return ret;
}

bool Grid::RemoveEntity(int entityID) {
    bool ret = false;
    auto it = m_entities.find(entityID);
    if (it == m_entities.end()) {
        LOG_DEBUG("grid remove entity fail, entityID: %d not found", entityID);
        goto Exit0;
    }

    m_entities.erase(entityID);

    ret = true;
Exit0:
    return ret;
}

bool Grid::BroadcastEntityEnter(Entity* entity) {
    for (auto pOther : m_entities) {
        Entity* other = pOther.second;
        other->OnEntityEnter(entity);
    }
    return true;
}

bool Grid::BroadcastEntityExit(Entity* entity) {
    for (auto pOther : m_entities) {
        Entity* other = pOther.second;
        other->OnEntityExit(entity);
    }
    return true;
}

bool Grid::BroadcastEntityMove(Entity* entity) {
    for (auto pOther : m_entities) {
        Entity* other = pOther.second;
        other->OnEntityMove(entity);
    }
    return true;
}