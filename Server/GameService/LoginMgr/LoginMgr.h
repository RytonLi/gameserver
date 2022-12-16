#pragma once

#include <unordered_map>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "Codec/Codec.h"
#include "LoginPlayerInfo.h"

GAME_SERVICE_NS_BEGIN

class LoginMgr : public IModule, public ISingleton<LoginMgr> {
public:
	bool Init() override;
	bool UnInit() override;
    bool Update() override;
    bool Quit() override;

private:
    //处理登出
    bool _HandleLogout(uint64_t clientID, const Packet* pack);
    //处理注册请求
    bool _HandleCreate(uint64_t clientID, const Packet* pack);
	//处理登录请求
	bool _HandleLogin(uint64_t clientID, const Packet* pack);
    //处理角色列表请求
    bool _HandleGetRoleList(uint64_t clientID, const Packet* pack);
    //处理创建角色请求
    bool _HandleAddRole(uint64_t clientID, const Packet* pack);
    //选择选择角色请求
    bool _HandleSelectRole(uint64_t clientID, const Packet* pack);

    bool _CheckIsLogin(uint64_t clientID, const std::string& playerID);
    bool _AddLoginPlayer(uint64_t clientID, const std::string& playerID);
    bool _RemoveLoginPlayer(LoginPlayerInfo* info);

    LoginPlayerInfo* _GetLoginPlayerInfo(uint64_t clientID);
    LoginPlayerInfo* _GetLoginPlayerInfo(const std::string& playerID);

    void _UpdateLoginInfoExpireTime(LoginPlayerInfo* info);

	//维护clientID与登录玩家
	std::unordered_map<uint64_t, LoginPlayerInfo*> m_clientIDToPlayers;
	//维护玩家ID与登录玩家，登录检测
	std::unordered_map<std::string, LoginPlayerInfo*> m_playerIDToPlayers;
};

GAME_SERVICE_NS_END