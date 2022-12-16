#pragma once

#include <vector>
#include <unordered_map>

#include "SceneConfig.h"
#include "Entity.h"
#include "Grid.h"

GAME_SERVICE_NS_BEGIN

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    bool Init(SceneConfig* config);
    bool UnInit();

    bool Update();
    bool LateUpdate();

    bool EnterScene(Entity* entity);
    bool ExitScene(int entityID);

    bool SyncMove(Entity* entity, const Position& desPos, bool flag);

    bool ChangeEntityEnterSight(const std::vector<int>& gridIndexs, Entity* entity);
    bool ChangeEntityExitSight(const std::vector<int>& gridIndexs, Entity* entity);

    bool BroadcastEnter(const std::vector<int>& gridIndexs, Entity* entity);
    bool BroadcastExit(const std::vector<int>& gridIndexs, Entity* entity);
    bool BroadcastMove(const std::vector<int>& gridIndexs, Entity* entity);

    int GetCol();
    int GetRow();

private:
    friend class SceneMgr;

    bool _InitGrid();

    bool _EnterGrid(int gridIndex, Entity* entity);
    bool _ExitGrid(int gridIndex, int entityID);

    bool _AddEntity(Entity* entity);
    bool _RemoveEntity(int entityID);

    int _GetGridIndex(const Position& pos);

    uint64_t m_lastUpdateTs;

    int m_unitSizeX;
    int m_unitSizeY;
    int m_col;
    int m_row;

    int m_id;
    int m_length;
    int m_width;

    RoleInitStatus m_roleInitStatus;

    std::unordered_map<int, Entity*> m_entities;
    std::vector<Grid*> m_grids;
};

GAME_SERVICE_NS_END