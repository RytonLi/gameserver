#include "NetPackDispatcher.h"

using namespace std;
using namespace gs;

bool NetPackDispatcher::Init() {
    m_handlers.resize(MY_UINT16_MAX);
    m_logoutHandlerFunc = nullptr;
	return true;
}

bool NetPackDispatcher::UnInit() {
	return true;
}

bool NetPackDispatcher::Dispatch(uint16_t cmd, uint64_t clientID, const Packet* pack) {
	bool ret = false;

    if (m_handlers[cmd]) {
        ret = m_handlers[cmd](clientID, pack);
    }
    else {
        LOG_DEBUG("on net pack dispatch, cmd: %d not found, client: %lld", pack->cmd, clientID);
    }

	return ret;
}

bool NetPackDispatcher::RegisterNetPackHandlerFunc(uint16_t cmd, NetPackHandlerFunc func) {
	bool ret = false;
	
    if (m_handlers[cmd]) {
        LOG_ERROR("register netpack handler func fail, cmd: %d existed\n", cmd);
    }
    else {
        m_handlers[cmd] = func;
        ret = true;
    }

	return ret;
}

bool NetPackDispatcher::RemoveNetPackHandlerFunc(uint16_t cmd) {
	bool ret = false;

    if (m_handlers[cmd]) {
        m_handlers[cmd] = nullptr;
        ret = true;
    }
    else {
        LOG_ERROR("Remove netpack handler func fail, cmd: %d not found\n", cmd);
    }

	return ret;
}

bool NetPackDispatcher::DispatchLogout(uint64_t clientID) {
    bool ret = false;

    if (m_logoutHandlerFunc) {
        ret = m_logoutHandlerFunc(clientID, nullptr);
    }
    else {
        LOG_DEBUG("on net pack dispatch, log out func not found, client: %lld", clientID);
    }

    return ret;
}

bool NetPackDispatcher::RegisterLogoutHandlerFunc(NetPackHandlerFunc func) {
    bool ret = false;

    if (m_logoutHandlerFunc) {
        LOG_ERROR("register log out handler func fail, existed\n");
    }
    else {
        m_logoutHandlerFunc = func;
        ret = true;
    }

    return ret;
}

bool NetPackDispatcher::RemoveLogoutHandlerFunc() {
    bool ret = false;

    if (m_logoutHandlerFunc) {
        m_logoutHandlerFunc = nullptr;
        ret = true;
    }
    else {
        LOG_ERROR("Remove log out handler func fail, not found\n");
    }

    return ret;
}