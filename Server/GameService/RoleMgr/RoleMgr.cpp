#include "RoleMgr.h"
#include "SceneMgr/SceneMgr.h"
#include "Proto/player.pb.h"
#include "NetMgr/NetMgr.h"
#include "NetPackDispatcher/NetPackDispatcher.h"
#include "ShopMgr/ShopMgr.h"
#include "FileDB/FileDB.h"

#define CHECK_TIMEOUT_RATE 60
#define ROLE_TIMEOUT 600

using namespace std;
using namespace gs;
using namespace TCCamp;

bool RoleMgr::Init() {
    m_clientIDToRoles.clear();
    m_roleIDToRoles.clear();

    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_UPDATE_MONEY_REQ,
        bind(&RoleMgr::_HandleUpdateMoney, this, placeholders::_1, placeholders::_2)
    );
    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_UPDATE_BAGLIST_REQ,
        bind(&RoleMgr::_HandleUpdateBagList, this, placeholders::_1, placeholders::_2)
    );
    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_UPDATE_SHOPLIST_REQ,
        bind(&RoleMgr::_HandleUpdateShopList, this, placeholders::_1, placeholders::_2)
    );
    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_ROLE_BUY_ITEM_REQ,
        bind(&RoleMgr::_HandleBuyItem, this, placeholders::_1, placeholders::_2)
    );
    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_ENTER_SCENE_REQ,
        bind(&RoleMgr::_HandleEnterScene, this, placeholders::_1, placeholders::_2)
    );
    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_ROLE_MOVE_SYNC_REQ,
        bind(&RoleMgr::_HandleMoveSync, this, placeholders::_1, placeholders::_2)
    );

    //³¬Ê±¼ì²â
    msm::ServiceMgr::Ins().AddTimer(
        CHECK_TIMEOUT_RATE * 1000,
        true,
        [this]() {
            for (auto it = m_roleIDToRoles.begin(); it != m_roleIDToRoles.end(); it++) {
                if (msm::ServiceMgr::Ins().Now() >= it->second->GetExpireTime()) {
                    NetMgr::Ins().CloseClient(it->second->GetClientID());
                }
            }
        }
    );

	return true;
}

bool RoleMgr::UnInit() {
	return true;
}

bool RoleMgr::Update() {
    return true;
}

bool RoleMgr::Quit() {
    return true;
}

bool RoleMgr::AddRole(uint64_t clientID, const string& playerID, const RoleData& roleData) {
    bool ret = false;
    Role* role = nullptr;

    if (_CheckIsSelectRole(clientID, roleData.id)) {
        goto Exit0;
    }
    
    role = new Role(clientID, playerID, roleData);
    m_clientIDToRoles.insert(pair<uint64_t, Role*>(clientID, role));
    m_roleIDToRoles.insert(pair<int, Role*>(roleData.id, role));
    
    ret = true;
Exit0:
    return ret;
}

bool RoleMgr::RemoveRole(uint64_t clientID) {
    Role* role = _GetRole(clientID);

    if (!role) {
        LOG_ERROR("on remove role, client: %lld not found", clientID);
        goto Exit0;
    }

    role->GetSceneRole()->ExitScene();

    FileDB::Ins().UpdateRoleDataByPlayerIDAndRoleID(role->GetPlayerID(), role->GetID(), role->GetRoleData());

    m_clientIDToRoles.erase(clientID);
    m_roleIDToRoles.erase(role->GetID());

    delete role;

Exit0:
    return true;
}

bool RoleMgr::_HandleUpdateMoney(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle update money req", clientID);
    Role* role = _GetRole(clientID);

    if (!role) {
        LOG_ERROR("client: %lld, on handle update money req, role not found", clientID);
        goto Exit0;
    }

    role->UpdateMoney();

Exit0:
    return true;
}

bool RoleMgr::_HandleUpdateBagList(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle update bag list req", clientID);
    Role* role = _GetRole(clientID);

    if (!role) {
        LOG_ERROR("client: %lld, on handle update bag list req, role not found", clientID);
        goto Exit0;
    }

    role->UpdateBagList();

Exit0:
    return true;
}

bool RoleMgr::_HandleUpdateShopList(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle update shop list req", clientID);
    Role* role = _GetRole(clientID);

    if (!role) {
        LOG_ERROR("client: %lld, on handle update shop list req, role not found", clientID);
        goto Exit0;
    }

    role->UpdateShopList();

Exit0:
    return true;
}

bool RoleMgr::_HandleBuyItem(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle buy item req", clientID);
    Role* role = _GetRole(clientID);
    RoleBuyItemReq req;

    if (!role) {
        LOG_ERROR("client: %lld, on handle buy item req, role not found", clientID);
        goto Exit0;
    }

    req.ParseFromArray(pack->data, pack->len);

    role->BuyItem(req.itemid(), req.itemcount());

Exit0:
    return true;
}

bool RoleMgr::_HandleEnterScene(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle enter scene req", clientID);
    Role* role = _GetRole(clientID);
    EnterSceneReq req;

    if (!role) {
        LOG_ERROR("client: %lld, on handle enter scene req, role not found", clientID);
        goto Exit0;
    }
    
    req.ParseFromArray(pack->data, pack->len);
    role->GetSceneRole()->EnterScene(req.sceneid());

Exit0:
    return true;
}

bool RoleMgr::_HandleMoveSync(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle move sync req", clientID);
    Role* role = _GetRole(clientID);
    RoleMoveSyncReq req;

    if (!role) {
        LOG_ERROR("client: %lld, on handle move sync req, role not found", clientID);
        goto Exit0;
    }

    req.ParseFromArray(pack->data, pack->len);

    role->GetSceneRole()->OnMoveSync(
        { req.status().status().pos().x(), req.status().status().pos().y() },
        { req.status().status().rot().x(), req.status().status().rot().y() },
        req.status().status().speed()
    );

Exit0:
    return true;
}

bool RoleMgr::_CheckIsSelectRole(uint64_t clientID, int roleID) {
    if (m_clientIDToRoles.count(clientID) ||
        m_roleIDToRoles.count(roleID)) {
        return true;
    }
    return false;
}

Role* RoleMgr::_GetRole(uint64_t clientID) {
    Role* role = nullptr;
    auto it = m_clientIDToRoles.find(clientID);
    if (it != m_clientIDToRoles.end()) {
        role = it->second;
        role->UpdateExpireTime(msm::ServiceMgr::Ins().Now() + ROLE_TIMEOUT * 1000);
    }
    return role;
}