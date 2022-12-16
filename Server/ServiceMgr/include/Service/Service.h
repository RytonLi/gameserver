#pragma once

#include "../UVInclude/uv.h"
#include "../TcpServer/TcpServer.h"
#include "ModuleMgr.h"

namespace msm {

class Service {
public:
	bool Init(uv_loop_t* loop, INetwork* network);
	bool UnInit();

    bool AddModule(IModule* module);

    bool Send(uint64_t sessionID, const char* buf, ssize_t size, AfterWriteCallback callback = nullptr, void* data = nullptr);
    bool Close(uint64_t sessionID);

private:
    friend class ServiceMgr;
    bool _Listen(const char* ip, int port);
    bool _Update();

    TcpServer* m_tcpServer;
    ModuleMgr* m_moduleMgr;
};

}