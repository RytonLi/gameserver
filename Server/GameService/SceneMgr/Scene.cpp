#include "Scene.h"
#include <ServiceMgr.h>
#include "Cube.h"

#define UNIT_SIZE_X 10
#define UNIT_SIZE_Y 10

using namespace std;
using namespace gs;

bool Scene::Init(SceneConfig* config) {
    m_id = config->id;
    m_length = config->length;
    m_width = config->width;
    _InitGrid();

    m_entities.clear();

    m_roleInitStatus = config->roleInitStatus;

    for (int i = 0; i < config->cubes.size(); i++) {
        const CubeInfo& info = config->cubes[i];
        Cube* cube = new Cube(emEntityCube, info.id, info.trans);
        EnterScene(cube);
    }

    m_lastUpdateTs = msm::ServiceMgr::Ins().Now();

    return true;
}

bool Scene::UnInit() {
    return true;
}

bool Scene::Update() {
    uint64_t diff = msm::ServiceMgr::Ins().Now() - m_lastUpdateTs;
    for (auto it = m_entities.begin(); it != m_entities.end(); it++) {
        it->second->Update(diff);
    }
    m_lastUpdateTs = msm::ServiceMgr::Ins().Now();

    LateUpdate();

    return true;
}

bool Scene::LateUpdate() {
    for (auto it = m_entities.begin(); it != m_entities.end(); it++) {
        it->second->LateUpdate();
    }
    return true;
}

bool Scene::EnterScene(Entity* entity) {
    if (entity->m_type == emEntityRole) {
        entity->m_trans = m_roleInitStatus.trans;
        entity->m_speed = m_roleInitStatus.speed;
    }
    return _AddEntity(entity);
}

bool Scene::ExitScene(int entityID) {
    return _RemoveEntity(entityID);
}

bool Scene::SyncMove(Entity* entity, const Position& desPos, bool flag) {
    int oldIndex = entity->m_aoi.m_centerGridIndex;
    int newIndex = _GetGridIndex(desPos);

    if (oldIndex != newIndex) {
        if (newIndex != -1) {
            _ExitGrid(oldIndex, entity->m_id);
            _EnterGrid(newIndex, entity);

            entity->m_trans.pos = desPos;

            entity->m_aoi.Update(newIndex);
        }
        else {
            const vector<int>& gridIndexs = entity->m_aoi.GetAOI();
            BroadcastMove(gridIndexs, entity);
        }
    }
    else {
        entity->m_trans.pos = desPos;
        if (flag) {
            const vector<int>& gridIndexs = entity->m_aoi.GetAOI();
            BroadcastMove(gridIndexs, entity);
        }
    }
    return true;
}

bool Scene::ChangeEntityEnterSight(const vector<int>& gridIndexs, Entity* entity) {
    for (int index : gridIndexs) {
        if (index >= 0 && index < m_grids.size()) {
            for (auto pOther : m_grids[index]->m_entities) {
                Entity* other = pOther.second;
                if (other->m_id != entity->m_id) {
                    entity->OnEntityEnter(other);
                }
            }
        }
    }
    return true;
}

bool Scene::ChangeEntityExitSight(const vector<int>& gridIndexs, Entity* entity) {
    for (int index : gridIndexs) {
        if (index >= 0 && index < m_grids.size()) {
            for (auto pOther : m_grids[index]->m_entities) {
                Entity* other = pOther.second;
                if (other->m_id != entity->m_id) {
                    entity->OnEntityExit(other);
                }
            }
        }
    }
    return true;
}

bool Scene::BroadcastEnter(const std::vector<int>& gridIndexs, Entity* entity) {
    for (int index : gridIndexs) {
        if (index >= 0 && index < m_grids.size()) {
            m_grids[index]->BroadcastEntityEnter(entity);
        }
    }
    return true;
}

bool Scene::BroadcastExit(const std::vector<int>& gridIndexs, Entity* entity) {
    for (int index : gridIndexs) {
        if (index >= 0 && index < m_grids.size()) {
            m_grids[index]->BroadcastEntityExit(entity);
        }
    }
    return true;
}

bool Scene::BroadcastMove(const std::vector<int>& gridIndexs, Entity* entity) {
    for (int index : gridIndexs) {
        if (index >= 0 && index < m_grids.size()) {
            m_grids[index]->BroadcastEntityMove(entity);
        }
    }
    return true;
}

int Scene::GetCol() {
    return m_col;
}

int Scene::GetRow() {
    return m_row;
}

bool Scene::_InitGrid() {
    m_unitSizeX = UNIT_SIZE_X;
    m_unitSizeY = UNIT_SIZE_Y;

    m_col = m_length % m_unitSizeX == 0 ? m_length / m_unitSizeX : m_length / m_unitSizeX + 1;
    m_row = m_width % m_unitSizeY == 0 ? m_width / m_unitSizeY : m_width / m_unitSizeY + 1;

    m_grids.clear();
    for (int i = 0; i < m_row * m_col; i++) {
        Grid* grid = new Grid;
        grid->Init(i);
        m_grids.push_back(grid);
    }
    return true;
}

bool Scene::_EnterGrid(int gridIndex, Entity* entity) {
    return m_grids[gridIndex]->AddEntity(entity);
}

bool Scene::_ExitGrid(int gridIndex, int entityID) {
    return m_grids[gridIndex]->RemoveEntity(entityID);
}

bool Scene::_AddEntity(Entity* entity) {
    bool ret = false;
    int gridIndex;
    
    if (m_entities.count(entity->m_id)) {
        LOG_DEBUG("scene add entity fail, entityID: %d exist", entity->m_id);
        goto Exit0;
    }

    gridIndex = _GetGridIndex(entity->m_trans.pos);
    if (gridIndex == -1) {
        LOG_DEBUG("scene add entity fail, entityID: %d pos out map", entity->m_id);
        goto Exit0;
    }

    if (!m_grids[gridIndex]->AddEntity(entity)) {
        LOG_DEBUG("scene add entity fail, entityID: %d add grid fail", entity->m_id);
        goto Exit0;
    }

    entity->m_scene = this;
    entity->m_aoi.Update(gridIndex);
    m_entities.insert(pair<int, Entity*>(entity->m_id, entity));

    ret = true;
Exit0:
    return ret;
}

bool Scene::_RemoveEntity(int entityID) {
    bool ret = false;
    Entity* entity = nullptr;

    auto it = m_entities.find(entityID);
    if (it == m_entities.end()) {
        LOG_DEBUG("scene remove entity fail, entityID: %d not found", entityID);
        goto Exit0;
    }

    entity = it->second;

    m_grids[entity->m_aoi.m_centerGridIndex]->RemoveEntity(entityID);

    entity->m_aoi.Update(-1);
    entity->m_scene = nullptr;
    m_entities.erase(it);

    ret = true;
Exit0:
    return ret;
}

int Scene::_GetGridIndex(const Position& pos) {
    int gridIndex = -1;
    if (pos.x >= 0 && pos.x < m_length &&
        pos.y >= 0 && pos.y < m_width) {
        gridIndex = m_col * (int)(pos.x / m_unitSizeX) + (int)(pos.y / m_unitSizeY);
    }
    return gridIndex;
}