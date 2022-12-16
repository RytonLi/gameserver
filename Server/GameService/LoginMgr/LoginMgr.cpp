#include "LoginMgr.h"
#include "Proto/player.pb.h"
#include "NetPackDispatcher/NetPackDispatcher.h"
#include "FileDB/FileDB.h"
#include "NetMgr/NetMgr.h"
#include "RoleMgr/RoleMgr.h"

#define CHECK_TIMEOUT_RATE 60
#define LOGIN_TIMEOUT 300

using namespace std;
using namespace TCCamp;
using namespace gs;

bool LoginMgr::Init() {
	m_clientIDToPlayers.clear();
	m_playerIDToPlayers.clear();

    NetPackDispatcher::Ins().RegisterLogoutHandlerFunc(
        bind(&LoginMgr::_HandleLogout, this, placeholders::_1, placeholders::_2)
    );

    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_CREATE_REQ,
        bind(&LoginMgr::_HandleCreate, this, placeholders::_1, placeholders::_2)
    );

	NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
		CLIENT_CMD::CLIENT_LOGIN_REQ, 
		bind(&LoginMgr::_HandleLogin, this, placeholders::_1, placeholders::_2)
	);

    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_GET_ROLELIST_REQ,
        bind(&LoginMgr::_HandleGetRoleList, this, placeholders::_1, placeholders::_2)
    );

    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_CREATE_ROLE_REQ,
        bind(&LoginMgr::_HandleAddRole, this, placeholders::_1, placeholders::_2)
    );

    NetPackDispatcher::Ins().RegisterNetPackHandlerFunc(
        CLIENT_CMD::CLIENT_GET_ROLEDATA_REQ,
        bind(&LoginMgr::_HandleSelectRole, this, placeholders::_1, placeholders::_2)
    );

    //超时检测
    msm::ServiceMgr::Ins().AddTimer(
        CHECK_TIMEOUT_RATE * 1000,
        true,
        [this]() {
            for (auto it = m_clientIDToPlayers.begin(); it != m_clientIDToPlayers.end(); it++) {
                if (!it->second->isSelectRole && msm::ServiceMgr::Ins().Now() >= it->second->expireTime) {
                    NetMgr::Ins().CloseClient(it->second->clientID);
                }
            }
        }
    );

	return true;
}

bool LoginMgr::UnInit() {
	return true;
}

bool LoginMgr::Update() {
    return true;
}

bool LoginMgr::Quit() {
    return true;
}

//处理登出
bool LoginMgr::_HandleLogout(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle log out", clientID);
    LoginPlayerInfo* info = _GetLoginPlayerInfo(clientID);
    if (!info) {
        LOG_DEBUG("client: %lld, not login in", clientID);
        goto Exit0;
    }
    
    if (!info->isSelectRole) {
        _RemoveLoginPlayer(info);
        goto Exit0;
    }

    RoleMgr::Ins().RemoveRole(clientID);
    _RemoveLoginPlayer(info);

Exit0:
    return true;
}

//处理注册请求
bool LoginMgr::_HandleCreate(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle create req", clientID);
    AccountCreateReq req;
    req.ParseFromArray(pack->data, pack->len);

    FileDB::Ins().CreatePlayer(
        req.accountid(), 
        req.password(),
        [clientID](FILEDB_RESULT res) {
            AccountCreateRsp rsp;
            if (res == emFileDBSuccess) {
                rsp.set_result(PB_RESULT_CODE::SUCCESS);
            }
            else if (res == emFileDBPlayerExist) {
                rsp.set_result(PB_RESULT_CODE::CREATE_ACCOUNT_EXIST);
            }
            else {
                rsp.set_result(PB_RESULT_CODE::SERVER_ERROR);
            }
            LOG_DEBUG("client: %lld, handle create req, result: %d", clientID, rsp.result());
            NetMgr::Ins().SendToClient(clientID, SERVER_CMD::SERVER_CREATE_RSP, &rsp);
        }
    );
    
    return true;

}

//处理登录请求
bool LoginMgr::_HandleLogin(uint64_t clientID, const Packet* pack) {
	LOG_DEBUG("client: %lld, start handle login req", clientID);
    AccountLoginReq req;
    req.ParseFromArray(pack->data, pack->len);

    if (_CheckIsLogin(clientID, req.accountid())) {
        AccountLoginRsp rsp;
        rsp.set_result(PB_RESULT_CODE::LOGIN_ACCOUNT_ONLINE);
        LOG_DEBUG("client: %lld, handle login req, result: %d", clientID, rsp.result());
        NetMgr::Ins().SendToClient(clientID, SERVER_CMD::SERVER_LOGIN_RSP, &rsp);
        goto Exit0;
    }
    
    FileDB::Ins().GetPasswordByPlayerID(
        req.accountid(),
        [clientID, req, this](FILEDB_RESULT res, const string& password) {
            AccountLoginRsp rsp;
            if (res == emFileDBSuccess) {
                if (req.password().compare(password) == 0) {
                    bool res = _AddLoginPlayer(clientID, req.accountid());
                    if (res) {
                        rsp.set_result(PB_RESULT_CODE::SUCCESS);
                        NetMgr::Ins().SetClientStatus(clientID, true);
                    }
                    else {
                        rsp.set_result(PB_RESULT_CODE::LOGIN_ACCOUNT_ONLINE);
                    }
                }
                else {
                    rsp.set_result(PB_RESULT_CODE::LOGIN_PASSWORD_ERROR);
                }
            }
            else if (res == emFileDBPlayerNotFound) {
                rsp.set_result(PB_RESULT_CODE::LOGIN_ACCOUNT_NOT_EXIST);
            }
            else {
                rsp.set_result(PB_RESULT_CODE::SERVER_ERROR);
            }
            LOG_DEBUG("client: %lld, handle login req, result: %d", clientID, rsp.result());
            NetMgr::Ins().SendToClient(clientID, SERVER_CMD::SERVER_LOGIN_RSP, &rsp);
        }
    );

Exit0:
	return true;
}

//处理角色列表请求
bool LoginMgr::_HandleGetRoleList(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle get role list req", clientID);
    LoginPlayerInfo* info = _GetLoginPlayerInfo(clientID);
    if (!info) {
        LOG_ERROR("client: %lld, handle get role list req, login info not found", clientID);
        goto Exit0;
    }

    _UpdateLoginInfoExpireTime(info);

    FileDB::Ins().GetAllRoleDataByPlayerID(
        info->playerID,
        [info](FILEDB_RESULT res, const vector<RoleData>& roleDatas) {
            GetRoleListRsp rsp;
            if (res == emFileDBSuccess) {
                for (auto roleData : roleDatas) {
                    rsp.add_roleids(roleData.id);
                }
                rsp.set_result(PB_RESULT_CODE::SUCCESS);
            }
            else {
                rsp.set_result(PB_RESULT_CODE::SERVER_ERROR);
            }
            LOG_DEBUG("client: %lld, handle get role list req, result: %d", info->clientID, rsp.result());
            NetMgr::Ins().SendToClient(info->clientID, SERVER_CMD::SERVER_GET_ROLELIST_RSP, &rsp);
        }
    );

Exit0:
    return true;
}

//处理创建角色请求
bool LoginMgr::_HandleAddRole(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle add role req", clientID);
    LoginPlayerInfo* info = _GetLoginPlayerInfo(clientID);
    if (!info) {
        LOG_ERROR("client: %lld, handle add role req, login info not found", clientID);
        goto Exit0;
    }

    _UpdateLoginInfoExpireTime(info);

    FileDB::Ins().AddRoleByPlayerID(
        info->playerID,
        [info](FILEDB_RESULT res) {
            RoleCreateRsp rsp;
            if (res == emFileDBSuccess) {
                rsp.set_result(PB_RESULT_CODE::SUCCESS);
            }
            else {
                rsp.set_result(PB_RESULT_CODE::CREATE_ROLE_COUNT_MAX);
            }
            LOG_DEBUG("client: %lld, handle add role req, result: %d", info->clientID, rsp.result());
            NetMgr::Ins().SendToClient(info->clientID, SERVER_CMD::SERVER_CREATE_ROLE_RSP, &rsp);
        }
    );

Exit0:
    return true;
}

void _InitPBRoleData(PBRoleData* pbData, const RoleData& roleData) {
    pbData->set_roleid(roleData.id);
    pbData->set_money(roleData.money);
    for (auto pItem : roleData.bagItems) {
        const BagItem& localItem = pItem.second;
        PBBagItem* pbItem = pbData->add_items();
        pbItem->set_id(localItem.id);
        pbItem->set_num(localItem.num);
    }
    pbData->set_sceneid(0);
    pbData->mutable_status()->mutable_pos()->set_x(0);
    pbData->mutable_status()->mutable_pos()->set_y(0);
    pbData->mutable_status()->mutable_rot()->set_x(1);
    pbData->mutable_status()->mutable_rot()->set_y(0);
    pbData->mutable_status()->set_speed(0);
    return;
}

//处理选择角色请求
bool LoginMgr::_HandleSelectRole(uint64_t clientID, const Packet* pack) {
    LOG_DEBUG("client: %lld, start handle select role req", clientID);
    GetRoleDataReq req;
    LoginPlayerInfo* info = _GetLoginPlayerInfo(clientID);
    if (!info) {
        LOG_ERROR("client: %lld, handle select role req, login info not found", clientID);
        goto Exit0;
    }

    _UpdateLoginInfoExpireTime(info);

    req.ParseFromArray(pack->data, pack->len);

    FileDB::Ins().GetRoleDataByPlayerIDAndRoleID(
        info->playerID,
        (int)req.roleid(),
        [info](FILEDB_RESULT res, const RoleData& roleData) {
            GetRoleDataRsp rsp;
            if (res == emFileDBSuccess) {
                bool res = RoleMgr::Ins().AddRole(info->clientID, info->playerID, roleData);
                if (res) {
                    info->isSelectRole = true;
                    rsp.set_result(PB_RESULT_CODE::SUCCESS);
                    _InitPBRoleData(rsp.mutable_data(), roleData);
                }
                else {
                    rsp.set_result(PB_RESULT_CODE::SERVER_ERROR);
                }
            }
            else {
                rsp.set_result(PB_RESULT_CODE::GET_ROLEDATA_ROLE_NOT_EXIST);
            }
            LOG_DEBUG("client: %lld, handle select role req, result: %d", info->clientID, rsp.result());
            NetMgr::Ins().SendToClient(info->clientID, SERVER_CMD::SERVER_GET_ROLEDATA_RSP, &rsp);
        }
    );

Exit0:
    return true;
}

bool LoginMgr::_CheckIsLogin(uint64_t clientID, const string& playerID) {
    if (m_clientIDToPlayers.count(clientID) || 
        m_playerIDToPlayers.count(playerID)) {
        return true;
    }
    return false;
}

bool LoginMgr::_AddLoginPlayer(uint64_t clientID, const std::string& playerID) {
    bool ret = false;
    LoginPlayerInfo* info = nullptr;

    if (_CheckIsLogin(clientID, playerID)) {
        LOG_ERROR("add login player fail, is logged in, clientID: %lld, playerID: %s", clientID, playerID.c_str());
        goto Exit0;
    }

    info = new LoginPlayerInfo(clientID, playerID);
    _UpdateLoginInfoExpireTime(info);

    m_clientIDToPlayers.insert(pair<uint64_t, LoginPlayerInfo*>(clientID, info));
    m_playerIDToPlayers.insert(pair<string, LoginPlayerInfo*>(playerID, info));

    ret = true;
Exit0:
    return ret;
}

bool LoginMgr::_RemoveLoginPlayer(LoginPlayerInfo* info) {
    m_clientIDToPlayers.erase(info->clientID);
    m_playerIDToPlayers.erase(info->playerID);
    delete info;
    return true;
}

LoginPlayerInfo* LoginMgr::_GetLoginPlayerInfo(uint64_t clientID) {
    LoginPlayerInfo* info = nullptr;
    auto it = m_clientIDToPlayers.find(clientID);
    if (it != m_clientIDToPlayers.end()) {
        info = it->second;
    }
    return info;
}

LoginPlayerInfo* LoginMgr::_GetLoginPlayerInfo(const string& playerID) {
    LoginPlayerInfo* info = nullptr;
    auto it = m_playerIDToPlayers.find(playerID);
    if (it != m_playerIDToPlayers.end()) {
        info = it->second;
    }
    return info;
}

void LoginMgr::_UpdateLoginInfoExpireTime(LoginPlayerInfo* info) {
    info->expireTime = msm::ServiceMgr::Ins().Now() + LOGIN_TIMEOUT * 1000;
}