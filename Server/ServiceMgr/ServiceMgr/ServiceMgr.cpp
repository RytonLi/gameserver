#include "ServiceMgr.h"

using namespace msm;

bool ServiceMgr::Init() {
	m_uvLoop     = new UVLoop;
    m_uvTimerMgr = new UVTimerMgr;
    m_uvSignalMgr = new UVSignalMgr;
    m_fileIOServer = new FileIOServer;

	m_uvLoop->Init();
    m_uvLoop->SetUpdateCallback(std::bind(
        &ServiceMgr::_Update, this
    ));
    m_uvTimerMgr->Init(m_uvLoop->Loop());
    m_uvSignalMgr->Init(m_uvLoop->Loop());
    m_fileIOServer->Init(m_uvLoop->Loop());
    m_services.clear();

	return true;
}

bool ServiceMgr::UnInit() {
	return true;
}

Service* ServiceMgr::AddService(class INetwork* network, const char* ip, int port) {
    Service* service = new Service;
    service->Init(m_uvLoop->Loop(), network);
    service->_Listen(ip, port);
    m_services.push_back(service);
	return service;
}

bool ServiceMgr::Start() {
    m_uvLoop->Start();
    return true;
}

bool ServiceMgr::Stop() {

    return true;
}

//------------------定时器----------------------//
int ServiceMgr::AddTimer(uint64_t timeout, bool repeat, TimeoutCallback callback) {
    return m_uvTimerMgr->AddTimer(timeout, repeat, callback);
}

bool ServiceMgr::RemoveTimer(int timerID) {
    return m_uvTimerMgr->RemoveTimer(timerID);
}

//------------------信号----------------------//
int ServiceMgr::AddSignalHandler(int signum, SignalCallback callback) {
    return m_uvSignalMgr->AddSignalHandler(signum, callback);
}

bool ServiceMgr::RemoveSignalHandler(int handlerID) {
    return m_uvSignalMgr->RemoveSignalHandler(handlerID);
}

//------------------文件-----------------------//
FILE_IO_RESULT ServiceMgr::Access(const char* path) {
    return m_fileIOServer->Access(path);
}

FILE_IO_RESULT ServiceMgr::Open(const char* path, int flags) {
    return m_fileIOServer->Open(path, flags);
}
FILE_IO_RESULT ServiceMgr::WriteAsync(const char* path, const char* data, int len, int offset, UVFileCallback callback) {
    return m_fileIOServer->WriteAsync(path, data, len ,offset, callback);

}

int ServiceMgr::WriteSync(const char* path, const char* data, int len, int offset) {
    return m_fileIOServer->WriteSync(path, data, len, offset);
}

FILE_IO_RESULT ServiceMgr::ReadAsync(const char* path, char* buf, int size, int offset, UVFileCallback callback) {
    return m_fileIOServer->ReadAsync(path, buf, size, offset, callback);
}

int ServiceMgr::ReadSync(const char* path, char* buf, int size, int offset) {
    return m_fileIOServer->ReadSync(path, buf, size, offset);
}

FILE_IO_RESULT ServiceMgr::Close(const char* path) {
    return m_fileIOServer->Close(path);

}

uint64_t ServiceMgr::GetFileSize(const char* path) {
    return m_fileIOServer->GetFileSize(path);
}

uint64_t ServiceMgr::Now() {
    return m_uvLoop->Now();
}

bool ServiceMgr::_Update() {
    for (Service* service : m_services) {
        service->_Update();
    }
    return true;
}