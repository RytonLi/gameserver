#pragma once

#include <unordered_map>
#include <functional>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "Role.h"
#include "Codec/Packet.h"

GAME_SERVICE_NS_BEGIN

class RoleMgr : public IModule, public ISingleton<RoleMgr> {
public:
	bool Init() override;
	bool UnInit() override;
    bool Update() override;
    bool Quit() override;

    bool AddRole(uint64_t clientID, const std::string& playerID, const RoleData& roleData);
    bool RemoveRole(uint64_t clientID);

private:
    bool _HandleUpdateMoney(uint64_t clientID, const Packet* pack);
    bool _HandleUpdateBagList(uint64_t clientID, const Packet* pack);
    bool _HandleUpdateShopList(uint64_t clientID, const Packet* pack);

    bool _HandleBuyItem(uint64_t clientID, const Packet* pack);

    bool _HandleEnterScene(uint64_t clientID, const Packet* pack);

    bool _HandleMoveSync(uint64_t clientID, const Packet* pack);

    bool _CheckIsSelectRole(uint64_t clientID, int roleID);

    Role* _GetRole(uint64_t clientID);

    std::unordered_map<uint64_t, Role*> m_clientIDToRoles;
	std::unordered_map<int, Role*> m_roleIDToRoles;
};

GAME_SERVICE_NS_END