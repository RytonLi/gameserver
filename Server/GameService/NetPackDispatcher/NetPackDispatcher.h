#pragma once

#include <vector>
#include <functional>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "Codec/Packet.h"

GAME_SERVICE_NS_BEGIN

using NetPackHandlerFunc = std::function<bool(uint64_t, const Packet*)>;

class NetPackDispatcher : public ISingleton<NetPackDispatcher> {
public:
	bool Init();
	bool UnInit();

	bool Dispatch(uint16_t cmd, uint64_t clientID, const Packet* pack);

	bool RegisterNetPackHandlerFunc(uint16_t cmd, NetPackHandlerFunc func);
	bool RemoveNetPackHandlerFunc(uint16_t cmd);

    bool DispatchLogout(uint64_t clientID);
    bool RegisterLogoutHandlerFunc(NetPackHandlerFunc func);
    bool RemoveLogoutHandlerFunc();
	
private:
    std::vector<NetPackHandlerFunc> m_handlers;
    NetPackHandlerFunc m_logoutHandlerFunc;
};

GAME_SERVICE_NS_END