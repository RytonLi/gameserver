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
    //����ǳ�
    bool _HandleLogout(uint64_t clientID, const Packet* pack);
    //����ע������
    bool _HandleCreate(uint64_t clientID, const Packet* pack);
	//�����¼����
	bool _HandleLogin(uint64_t clientID, const Packet* pack);
    //�����ɫ�б�����
    bool _HandleGetRoleList(uint64_t clientID, const Packet* pack);
    //��������ɫ����
    bool _HandleAddRole(uint64_t clientID, const Packet* pack);
    //ѡ��ѡ���ɫ����
    bool _HandleSelectRole(uint64_t clientID, const Packet* pack);

    bool _CheckIsLogin(uint64_t clientID, const std::string& playerID);
    bool _AddLoginPlayer(uint64_t clientID, const std::string& playerID);
    bool _RemoveLoginPlayer(LoginPlayerInfo* info);

    LoginPlayerInfo* _GetLoginPlayerInfo(uint64_t clientID);
    LoginPlayerInfo* _GetLoginPlayerInfo(const std::string& playerID);

    void _UpdateLoginInfoExpireTime(LoginPlayerInfo* info);

	//ά��clientID���¼���
	std::unordered_map<uint64_t, LoginPlayerInfo*> m_clientIDToPlayers;
	//ά�����ID���¼��ң���¼���
	std::unordered_map<std::string, LoginPlayerInfo*> m_playerIDToPlayers;
};

GAME_SERVICE_NS_END