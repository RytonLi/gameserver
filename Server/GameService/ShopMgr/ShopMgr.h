#pragma once

#include <unordered_map>
#include <functional>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "ShopItem.h"
#include "Codec/Packet.h"
#include "Proto/player.pb.h"

GAME_SERVICE_NS_BEGIN

class ShopMgr : public IModule, public ISingleton<ShopMgr> {
public:
	bool Init() override;
	bool UnInit() override;
    bool Update() override;
    bool Quit() override;

    void GetShopList(std::function<void(TCCamp::PB_RESULT_CODE, const std::unordered_map<int, ShopItem>&)> callback);
    void BuyItem(int itemID, int buyCount, int money, int personalLimit, std::function<void(TCCamp::PB_RESULT_CODE, int, int)> callback);

private:

    bool _LoadShopList();
    bool _ParseShopList(const char* data, int len);

    std::unordered_map<int, ShopItem> m_shopItems;

};

GAME_SERVICE_NS_END