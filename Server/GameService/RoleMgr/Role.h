#pragma once

#include <google/protobuf/message.h>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "DataStruct/PlayerData.h"
#include "SceneMgr/SceneRole.h"

GAME_SERVICE_NS_BEGIN

class Role {
public:
    Role(uint64_t clientID, const std::string& playerID, const RoleData& data) :
        m_clientID(clientID),
        m_playerID(playerID),
        m_data(data),
        m_sceneRole(clientID, data.id) {}
	virtual ~Role() = default;

    //更新金钱
    bool UpdateMoney();
    //更新背包
    bool UpdateBagList();
    //更新商店列表
    bool UpdateShopList();
    //购买物品
    bool BuyItem(int itemID, int buyCount);
    
    //----------------访问数据-----------------
    const std::string& GetPlayerID() const;
    int GetID() const;
    uint64_t GetClientID() const;
    const RoleData& GetRoleData() const;

    SceneRole* GetSceneRole();

    void UpdateExpireTime(uint64_t timestamp);
    uint64_t GetExpireTime();

    bool SendToClient(int cmd, google::protobuf::Message* msg);

private:
    int _GetBuyLimitByID(int itemID);
    void _SetBuyLimitByID(int itemID, int limit);
    void _AddBagItem(int itemID, int itemNum);

    uint64_t    m_clientID;
    std::string m_playerID;
    RoleData    m_data;

    SceneRole m_sceneRole;

    uint64_t m_expireTime;
};

GAME_SERVICE_NS_END