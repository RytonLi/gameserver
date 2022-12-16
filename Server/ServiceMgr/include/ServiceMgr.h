#pragma once

#include <vector>
#include <functional>

#include "Interface/ISingleton.h"
#include "Interface/IModule.h"
#include "Interface/INetwork.h"
#include "Log/Log.h"
#include "UVLoop/UVLoop.h"
#include "UVTimerMgr/UVTimerMgr.h"
#include "UVSignalMgr/UVSignalMgr.h"
#include "FileIOServer/FileIOServer.h"
#include "Service/Service.h"

namespace msm {

class ServiceMgr : public ISingleton<ServiceMgr> {
public:
	bool Init();
	bool UnInit();

	Service* AddService(class INetwork* network, const char* ip, int port);

	bool Start();
	bool Stop();

    //------------------��ʱ��----------------------//
    int AddTimer(uint64_t timeout, bool repeat, TimeoutCallback callback);
    bool RemoveTimer(int timerID);

    //------------------�ź�----------------------//
    int AddSignalHandler(int signum, SignalCallback callback);
    bool RemoveSignalHandler(int handlerID);

    //------------------�ļ�-----------------------//
    FILE_IO_RESULT Access(const char* path);
    FILE_IO_RESULT Open(const char* path, int flags);
    FILE_IO_RESULT WriteAsync(const char* path, const char* data, int len, int offset, UVFileCallback callback);
    int WriteSync(const char* path, const char* data, int len, int offset);
    FILE_IO_RESULT ReadAsync(const char* path, char* buf, int size, int offset, UVFileCallback callback);
    int ReadSync(const char* path, char* buf, int size, int offset);
    FILE_IO_RESULT Close(const char* path);

    uint64_t GetFileSize(const char* path); //-1��ʾʧ��

    uint64_t Now();

private:
    bool _Update();

	UVLoop*         m_uvLoop;
	UVTimerMgr*     m_uvTimerMgr;
    UVSignalMgr*    m_uvSignalMgr;
    FileIOServer*   m_fileIOServer;
    std::vector<Service*> m_services;
};

}