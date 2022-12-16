#pragma once

#include <unordered_map>
#include <functional>

#include "Utils/Define.h"
#include "UVInclude/uv.h"

MY_SERVICE_MGR_NS_BEGIN

using SignalCallback = std::function<void(int)>;

class UVSignalMgr {
public:
    UVSignalMgr() = default;
    virtual ~UVSignalMgr() = default;

    bool Init(uv_loop_t* loop);
    bool UnInit();

    //添加定时器，返回定时器ID
    int AddSignalHandler(int signum, SignalCallback callback);

    //移除定时器
    bool RemoveSignalHandler(int handlerID);

private:
    struct HandlerMsg {
        uv_signal_t     uvHandler;
        UVSignalMgr*    thisPtr;
        int             handlerID;
        SignalCallback callback;

        HandlerMsg(UVSignalMgr* thisPtr, int handlerID, SignalCallback callback) :
            thisPtr(thisPtr),
            handlerID(handlerID),
            callback(callback) {}
    };

private:
    static void _OnSignal(uv_signal_t* handle, int signum);
    void _CloseHandler(int handlerID);

    uv_loop_t* m_loop;
    int        m_autoIncID;

    std::unordered_map<int, HandlerMsg*> m_handlerMap;
};

MY_SERVICE_MGR_NS_END