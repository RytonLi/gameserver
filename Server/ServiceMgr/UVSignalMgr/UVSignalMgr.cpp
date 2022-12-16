#include "UVSignalMgr.h"

using namespace msm;
using namespace std;

bool UVSignalMgr::Init(uv_loop_t* loop) {
    m_loop = loop;
    m_autoIncID = 1;
    m_handlerMap.clear();
    return true;
}

bool UVSignalMgr::UnInit() {
    return true;
}

int UVSignalMgr::AddSignalHandler(int signum, SignalCallback callback) {
    int handlerID = m_autoIncID++;
    HandlerMsg* msg = new HandlerMsg(this, handlerID, callback);

    ::uv_signal_init(m_loop, (uv_signal_t*)msg);
    m_handlerMap.insert(pair<int, HandlerMsg*>(handlerID, msg));
    ::uv_signal_start((uv_signal_t*)msg, _OnSignal, signum);

    return handlerID;
}

bool UVSignalMgr::RemoveSignalHandler(int handlerID) {
    _CloseHandler(handlerID);
    return true;
}

void UVSignalMgr::_OnSignal(uv_signal_t* handle, int signum) {
    HandlerMsg* msg = (HandlerMsg*)handle;

    msg->callback(signum);
}

void UVSignalMgr::_CloseHandler(int handlerID) {
    HandlerMsg* msg = nullptr;
    auto it = m_handlerMap.find(handlerID);
    if (it == m_handlerMap.end()) {
        return;
    }

    msg = it->second;
    m_handlerMap.erase(it);

    if (::uv_is_active((uv_handle_t*)msg)) {
        ::uv_signal_stop((uv_signal_t*)msg);
    }

    if (::uv_is_closing((uv_handle_t*)msg) == 0) {
        ::uv_close((uv_handle_t*)msg,
            [](uv_handle_t* handle) { delete handle; });
    }
}