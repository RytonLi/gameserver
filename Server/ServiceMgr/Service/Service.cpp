#include "Service.h"

using namespace msm;

bool Service::Init(uv_loop_t* loop, INetwork* network) {
    m_tcpServer = new TcpServer;
    m_moduleMgr = new ModuleMgr;

    m_tcpServer->Init(loop, network);
    m_moduleMgr->Init();
	return true;
}

bool Service::UnInit() {
	return true;
}

bool Service::AddModule(IModule* module) {
    m_moduleMgr->AddModule(module);
    return true;
}

bool Service::Send(uint64_t sessionID, const char* buf, ssize_t size, AfterWriteCallback callback, void* data) {
    m_tcpServer->Send(sessionID, buf, size, callback, data);
    return true;
}

bool Service::Close(uint64_t sessionID) {
    m_tcpServer->CloseSession(sessionID);
    return true;
}

bool Service::_Listen(const char* ip, int port) {
    m_tcpServer->BindAndListen(ip, port);
    return true;
}

bool Service::_Update() {
    m_moduleMgr->Update();
    return true;
}