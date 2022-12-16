#include "SceneRole.h"
#include <cmath>
#include "SceneMgr/SceneMgr.h"
#include "NetMgr/NetMgr.h"
#include "SceneMgr/Cube.h"

#define MAX_SYNC_ENTITY_NUM 100

using namespace gs;
using namespace std;
using namespace TCCamp;

//进入场景
bool SceneRole::EnterScene(int sceneID) {
    bool ret = false;
    Scene* scene = SceneMgr::Ins().GetScene(sceneID);
    if (!scene) {
        LOG_DEBUG("enter scene fail, sceneID: %d not found", sceneID);
        goto Exit0;
    }

    if (m_scene) {
        LOG_DEBUG("enter scene fail, roleID: %d already in scene", m_id);
        goto Exit0;
    }

    ret = scene->EnterScene(this);
    if (ret) {
        EnterSceneRsp rsp;
        rsp.set_result(PB_RESULT_CODE::SUCCESS);
        _SendToClient(SERVER_CMD::SERVER_ENTER_SCENE_RSP, &rsp);
    }

Exit0:
    return ret;
}

//离开场景
bool SceneRole::ExitScene() {
    if (m_scene) {
        LOG_DEBUG("roleID: %d exit scene", m_id);
        m_scene->ExitScene(m_id);
    }
    return true;
}

bool SceneRole::OnMoveSync(Position pos, Rotation rot, int speed) {
    bool ret = false;
    RoleMoveSyncRsp rsp;

    if (!m_scene) {
        goto Exit0;
    }

    LOG_DEBUG("on move sync, roleID: %d\n\
        client: pos: (%f, %f), rot: (%f, %f), angle: %d, speed: %d\n\
        server: pos: (%f, %f), rot: (%f, %f), speed: %d",
        m_id,
        pos.x, pos.y, rot.x, rot.y, (int)(::acos(rot.x) / 3.14 * 180), speed,
        m_trans.pos.x, m_trans.pos.y, m_trans.rot.x, m_trans.rot.y, m_speed
    );

    if (rot.x * rot.x + rot.y * rot.y - 1 > 0.00001) {
        LOG_DEBUG("roleID: %d, enter invalid", m_id);
        goto Exit0;
    }

    m_trans.rot = rot;

    if (speed >= 0 && speed <= 5) {
        m_speed = speed;
    }

    m_isStatusChange = true;

    rsp.set_result(PB_RESULT_CODE::SUCCESS);
    _SendToClient(SERVER_CMD::SERVER_ROLE_MOVE_SYNC_RSP, &rsp);

    ret = true;
Exit0:
    return ret;
}

bool SceneRole::Update(uint64_t diff) {
    Position des = m_trans.pos;

    if (m_speed > 0) {
        double span = (double)m_speed * diff / 1000;
        des.x = m_trans.pos.x + m_trans.rot.x * span;
        des.y = m_trans.pos.y + m_trans.rot.y * span;
    }

    if (m_scene) {
        m_scene->SyncMove(this, des, m_isStatusChange);
    }

    m_isStatusChange = false;

    return true;
}

bool SceneRole::LateUpdate() {
    if (m_syncAnnounce.datas_size() != 0 ||
        m_syncAnnounce.cubeinfos_size() != 0) {
        _SendToClient(SERVER_CMD::SERVER_ROLE_MOVE_SYNC_ANNOUNCE, &m_syncAnnounce);
        m_syncAnnounce.clear_datas();
        m_syncAnnounce.clear_cubeinfos();
    }
    if (m_exitAnnounce.roleids_size() != 0 ||
        m_exitAnnounce.cubeids_size() != 0) {
        _SendToClient(SERVER_CMD::SERVER_ROLE_EXIT_SCENE_ANNOUNCE, &m_exitAnnounce);
        m_exitAnnounce.clear_roleids();
        m_exitAnnounce.clear_cubeids();
    }
    return true;
}

bool SceneRole::OnEntityEnter(Entity* entity) {
    //LOG_DEBUG("roleID: %d, entityID: %d enter", m_id, entity->m_id);
    if (entity->m_type == emEntityCube) {
        m_entities.insert(pair<int, Entity*>(entity->m_id, entity));

        CubeTransformInfo* info = m_syncAnnounce.add_cubeinfos();

        info->set_id(entity->m_id);

        info->mutable_pos()->set_x(entity->m_trans.pos.x);
        info->mutable_pos()->set_y(entity->m_trans.pos.y);

        info->mutable_rot()->set_x(entity->m_trans.rot.x);
        info->mutable_rot()->set_y(entity->m_trans.rot.y);

        info->mutable_scale()->set_x(entity->m_trans.scale.x);
        info->mutable_scale()->set_y(entity->m_trans.scale.y);
    }
    else if (entity->m_type == emEntityRole) {
        if (m_entities.size() < MAX_SYNC_ENTITY_NUM) {
            m_entities.insert(pair<int, Entity*>(entity->m_id, entity));

            PBRoleMoveSyncData* data = m_syncAnnounce.add_datas();

            data->set_roleid(entity->m_id);

            data->mutable_status()->mutable_pos()->set_x(entity->m_trans.pos.x);
            data->mutable_status()->mutable_pos()->set_y(entity->m_trans.pos.y);

            data->mutable_status()->mutable_rot()->set_x(entity->m_trans.rot.x);
            data->mutable_status()->mutable_rot()->set_y(entity->m_trans.rot.y);

            data->mutable_status()->set_speed(entity->m_speed);

        }
    }
    return true;
}

bool SceneRole::OnEntityExit(Entity* entity) {
    //LOG_DEBUG("roleID: %d, entityID: %d exit", m_id, entity->m_id);

    if (m_entities.count(entity->m_id)) {

        m_entities.erase(entity->m_id);

        if (entity->m_type == emEntityCube) {
            m_exitAnnounce.add_cubeids(entity->m_id);
        }
        else if (entity->m_type == emEntityRole) {
            m_exitAnnounce.add_roleids(entity->m_id);
        }

    }
    
    return true;
}

bool SceneRole::OnEntityMove(Entity* entity) {
    //LOG_DEBUG("roleID: %d, entityID: %d move", m_id, entity->m_id);

    if (m_entities.count(entity->m_id)) {

        if (entity->m_type == emEntityCube) {
            CubeTransformInfo* info = m_syncAnnounce.add_cubeinfos();

            info->set_id(entity->m_id);

            info->mutable_pos()->set_x(entity->m_trans.pos.x);
            info->mutable_pos()->set_y(entity->m_trans.pos.y);

            info->mutable_rot()->set_x(entity->m_trans.rot.x);
            info->mutable_rot()->set_y(entity->m_trans.rot.y);

            info->mutable_scale()->set_x(entity->m_trans.scale.x);
            info->mutable_scale()->set_y(entity->m_trans.scale.y);
        }
        else if (entity->m_type == emEntityRole) {
            PBRoleMoveSyncData* data = m_syncAnnounce.add_datas();

            data->set_roleid(entity->m_id);

            data->mutable_status()->mutable_pos()->set_x(entity->m_trans.pos.x);
            data->mutable_status()->mutable_pos()->set_y(entity->m_trans.pos.y);

            data->mutable_status()->mutable_rot()->set_x(entity->m_trans.rot.x);
            data->mutable_status()->mutable_rot()->set_y(entity->m_trans.rot.y);

            data->mutable_status()->set_speed(entity->m_speed);
        }

    }
    return true;
}

bool SceneRole::_SendToClient(int cmd, google::protobuf::Message* msg) {
    return NetMgr::Ins().SendToClient(m_clientID, cmd, msg);
}