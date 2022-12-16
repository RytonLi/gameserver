#include "Role.h"
#include "NetMgr/NetMgr.h"
#include "Proto/player.pb.h"
#include "ShopMgr/ShopMgr.h"
#include "SceneMgr/SceneMgr.h"

using namespace std;
using namespace gs;
using namespace TCCamp;

//----------------场景外-----------------
//更新金钱
bool Role::UpdateMoney() {
    UpdateMoneyRsp rsp;

    rsp.set_result(PB_RESULT_CODE::SUCCESS);
    rsp.set_money(m_data.money);

    LOG_DEBUG("client: %lld, handle update money req, result: %d", m_clientID, rsp.result());
    SendToClient(SERVER_CMD::SERVER_UPDATE_MONEY_RSP, &rsp);
    return true;
}

//更新背包
bool Role::UpdateBagList() {
    UpdateBagListRsp rsp;

    rsp.set_result(PB_RESULT_CODE::SUCCESS);
    
    for (auto pItem : m_data.bagItems) {
        const BagItem& item = pItem.second;
        PBBagItem* addItem = rsp.add_items();
        addItem->set_id(item.id);
        addItem->set_num(item.num);
    }

    LOG_DEBUG("client: %lld, handle update bag list req, result: %d", m_clientID, rsp.result());
    SendToClient(SERVER_CMD::SERVER_UPDATE_BAGLIST_RSP, &rsp);
    return true;
}

//更新商店列表
bool Role::UpdateShopList() {
    ShopMgr::Ins().GetShopList(
        [this](PB_RESULT_CODE res, const unordered_map<int, ShopItem>& shopList) {
            UpdateShopListRsp rsp;
            rsp.set_result(res);
            if (res == PB_RESULT_CODE::SUCCESS) {
                for (auto pItem : shopList) {
                    const ShopItem& item = pItem.second;
                    PBShopItem* addItem = rsp.add_items();
                    addItem->set_id(item.id);
                    addItem->set_price(item.price);
                    addItem->set_personallimitmaxcount(item.personalLimitMax);

                    //个人限购
                    if (item.personalLimitMax != -1) {
                        int limit = _GetBuyLimitByID(item.id);
                        if (limit == -1) {
                            addItem->set_personallimitcount(item.personalLimitMax);
                            _SetBuyLimitByID(item.id, item.personalLimitMax);
                        }
                        else {
                            addItem->set_personallimitcount(limit);
                        }
                    }

                    addItem->set_globallimitmaxcount(item.globalLimitMax);
                    addItem->set_globallimitcount(item.globalLimit);
                    addItem->set_starttime(item.startTime == 0 ? -1 : item.startTime);
                    addItem->set_endtime(item.endTime == 0 ? -1 : item.endTime);
                }
            }
            
            LOG_DEBUG("client: %lld, handle update shop list req, result: %d", m_clientID, rsp.result());
            SendToClient(SERVER_CMD::SERVER_UPDATE_SHOPLIST_RSP, &rsp);
        }
    );

    return true;
}

//购买物品
bool Role::BuyItem(int itemID, int buyCount) {
    int limit;

    if (buyCount <= 0) {
        goto Exit0;
    }

    limit = _GetBuyLimitByID(itemID);
    if (limit != -1 && buyCount > limit) {
        RoleBuyItemRsp rsp;
        rsp.set_result(PB_RESULT_CODE::SHOP_BUY_ITEM_PERSONAL_SHORTAGE);
        LOG_DEBUG("client: %lld, handle buy item req, result: %d", m_clientID, rsp.result());
        SendToClient(SERVER_CMD::SERVER_ROLE_BUY_ITEM_RSP, &rsp);
        goto Exit0;
    }

    ShopMgr::Ins().BuyItem(
        itemID,
        buyCount,
        m_data.money,
        limit,
        [itemID, buyCount, this](PB_RESULT_CODE res, int money, int personalLimit) {
            RoleBuyItemRsp rsp;
            rsp.set_result(res);
            if (res == PB_RESULT_CODE::SUCCESS) {
                _AddBagItem(itemID, buyCount);
                m_data.money = money;
                if (personalLimit != -1) {
                    _SetBuyLimitByID(itemID, personalLimit);
                }
            }
            LOG_DEBUG("client: %lld, handle buy item req, result: %d", m_clientID, rsp.result());
            SendToClient(SERVER_CMD::SERVER_ROLE_BUY_ITEM_RSP, &rsp);
        }
    );

Exit0:
    return true;
}

//----------------访问数据-----------------
const std::string& Role::GetPlayerID() const {
    return m_playerID;
}

int Role::GetID() const {
    return m_data.id;
}

uint64_t Role::GetClientID() const {
    return m_clientID;
}

const RoleData& Role::GetRoleData() const {
    return m_data;
}

SceneRole* Role::GetSceneRole() {
    return &m_sceneRole;
}

void Role::UpdateExpireTime(uint64_t timestamp) {
    m_expireTime = timestamp;
}

uint64_t Role::GetExpireTime() {
    return m_expireTime;
}

bool Role::SendToClient(int cmd, google::protobuf::Message* msg) {
    return NetMgr::Ins().SendToClient(m_clientID, cmd, msg);
}

int Role::_GetBuyLimitByID(int itemID) {
    auto it = m_data.buyLimits.find(itemID);
    if (it == m_data.buyLimits.end()) {
        return -1;
    }
    else {
        return it->second.num;
    }
}

void Role::_SetBuyLimitByID(int itemID, int limit) {
    auto it = m_data.buyLimits.find(itemID);
    if (it == m_data.buyLimits.end()) {
        m_data.buyLimits.insert(pair<int, BuyLimit>(itemID, { itemID, limit }));

    }
    else {
        it->second.num = limit;
    }
    return;
}

void Role::_AddBagItem(int itemID, int itemNum) {
    auto it = m_data.bagItems.find(itemID);
    if (it == m_data.bagItems.end()) {
        m_data.bagItems.insert(pair<int, BagItem>(itemID, { itemID, itemNum }));
    }
    else {
        it->second.num += itemNum;
    }
    return;
}
