#pragma once

#include <unordered_map>

#include "Session.h"
#include "Interface/INetwork.h"

MY_SERVICE_MGR_NS_BEGIN

using DefaultCallback            = std::function<void()>;
using OnConnectionStatusCallback = std::function<void(uint64_t)>;

class TcpServer {
public:
	TcpServer() = default;
	~TcpServer() = default;

	//初始化
	bool Init(uv_loop_t* loop, INetwork* network);
	bool UnInit();
	
	//绑定socket开始监听
	bool BindAndListen(const char* ip, int port);
	//关闭serverSocket，以及所有连接
	void Close(DefaultCallback callback);

	Session* GetSession(uint64_t sessionID);
	//关闭一个连接，主动调用/connection onclose的回调
	void CloseSession(uint64_t sessionID);
	//关闭所有连接
	void CloseAllSession();

	//向连接发送数据，可设置一个带参的回调
	bool Send(uint64_t sessionID, const char* buf, ssize_t size, AfterWriteCallback callback = nullptr, void* data = nullptr);

private:
	void _OnNewSession(int status);
	void _OnCloseComplete();
	void _OnMessage(uint64_t sessionID, const char* data, ssize_t size);

	void _RemoveConnection(uint64_t sessionID);

	uv_loop_t* m_loop;
	uv_tcp_t   m_uvServer;
	uint64_t   m_autoIncID;
    bool       m_isClosed;

    std::unordered_map<uint64_t, Session*> m_sessions;

	DefaultCallback m_onCloseCompletCallback;

    INetwork* m_network;
};

MY_SERVICE_MGR_NS_END