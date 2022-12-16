#include "TcpServer.h"
#include "Log/Log.h"

using namespace std;
using namespace msm;

bool TcpServer::Init(uv_loop_t* loop, class INetwork* network) {
	m_loop          = loop;
    m_network       = network;
	m_autoIncID     = 1;
    m_isClosed      = true;
    m_uvServer.data = this;
    m_sessions.clear();
	return true;
}

bool TcpServer::UnInit() {
	return true;
}

bool TcpServer::BindAndListen(const char* ip, int port) {
	bool ret   = false;
	int result = 0;
	sockaddr_in addr;

	result = uv_tcp_init(m_loop, &m_uvServer);
	if (result != 0) {
		LOG_ERROR("uv_tcp_init error %s", uv_strerror(result));
		goto Exit0;
	}

	result = uv_ip4_addr(ip, port, &addr);
	if (result != 0) {
		LOG_ERROR("uv_ip4_addr error %s", uv_strerror(result));
		goto Exit0;
	}

	result = uv_tcp_bind(&m_uvServer, (const struct sockaddr*)&addr, 0);
	if (result != 0) {
		LOG_ERROR("uv_tcp_bind error %s", uv_strerror(result));
		goto Exit0;
	}

	result = uv_listen((uv_stream_t*)&m_uvServer, SOMAXCONN, 
		//匿名函数，回调到成员函数_OnNewConnection
		[](uv_stream_t *server, int status) {
			TcpServer* thisPtr = (TcpServer*)server->data;
			thisPtr->_OnNewSession(status);
		});

	if (result != 0) {
		LOG_ERROR("uv_listen error %s\n", uv_strerror(result));
		goto Exit0;
	}

	m_isClosed = false;
	ret = true;
Exit0:
	if (!ret) {
		if (::uv_is_closing((uv_handle_t*)&m_uvServer) == 0) {
			::uv_close((uv_handle_t*)&m_uvServer, NULL);
		}
	}
	return ret;
}

//关闭serverSocket，以及所有连接
void TcpServer::Close(DefaultCallback callback) {
	m_isClosed = true;
	m_onCloseCompletCallback = callback;
	uv_tcp_t* ptr = &m_uvServer;
	if (::uv_is_active((uv_handle_t*)ptr)) {
		::uv_read_stop((uv_stream_t*)ptr);
	}
	if (::uv_is_closing((uv_handle_t*)ptr) == 0) {
		::uv_close((uv_handle_t*)ptr,
			//匿名函数，关闭server socket后，关闭所有连接
			[](uv_handle_t* handle) {
				TcpServer* thisPtr = (TcpServer*)handle->data;
				thisPtr->CloseAllSession();
			});
	}
}

Session* TcpServer::GetSession(uint64_t sessionID) {
    Session* session = nullptr;
	auto it = m_sessions.find(sessionID);
	if (it != m_sessions.end()) {
        session = it->second;
	}
	return session;
}

//关闭一个连接，主动调用/connection onclose的回调
void TcpServer::CloseSession(uint64_t sessionID) {
    Session* session = GetSession(sessionID);
	if (nullptr != session) {
        session->Close(
			//匿名函数，一个连接close complete回调
			[this](uint64_t sessionID) {
                Session* session = GetSession(sessionID);
				if (nullptr != session) {
                    m_network->OnCloseSession(sessionID);

					_RemoveConnection(sessionID);
				
					//检查是否Close完成
					if (m_isClosed && m_sessions.size() == 0) {
						_OnCloseComplete();
					}

					delete session;
				}
			});
	}
}

void TcpServer::CloseAllSession() {
	if (m_isClosed && m_sessions.size() == 0) {
		_OnCloseComplete();
	}

	for (auto& session : m_sessions) {
        session.second->OnSessionClose();
	}
}

//向连接发送数据，可设置一个带参的回调
bool TcpServer::Send(uint64_t sessionID, const char* buf, ssize_t size, AfterWriteCallback callback, void* data) {
	bool ret = false;
	Session* session = GetSession(sessionID);
	if (nullptr != session) {
		ret = session->Send(buf, size, callback, data);
	}
	return ret;
}

void TcpServer::_OnNewSession(int status) {
    Session* session = nullptr;
    uint64_t sessionID = m_autoIncID++;

	if (status < 0) {
		LOG_ERROR("new connection error %s\n", uv_strerror(status));
		goto Exit0;
	}

    session = new Session;
    session->Init(sessionID, m_loop);

	//accept
	if (uv_accept((uv_stream_t*)&m_uvServer, (uv_stream_t*)session->Handle()) != 0) {
		LOG_ERROR("on new connection, accept failed");
		goto Exit0;
	}
	
	//建立连接
	if (!session->Start()) {
		LOG_ERROR("on new connection, start read fail");
		goto Exit0;
	}

	//应用层回调
    m_network->OnNewSession(sessionID);
	
    session->SetMessageCallback(bind(&TcpServer::_OnMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));
    session->SetConnectCloseCallback(bind(&TcpServer::CloseSession, this, placeholders::_1));
	m_sessions.insert(pair<uint64_t, Session*>(sessionID, session));

    session = nullptr;
Exit0:
	if (session) {
        session->Close(
			[session](int) {
				delete session;
			}
		);
	}
	return;
}

void TcpServer::_OnCloseComplete() {
	if (m_onCloseCompletCallback) {
		m_onCloseCompletCallback();
	}
}

void TcpServer::_OnMessage(uint64_t sessionID, const char* data, ssize_t size) {
    m_network->OnMessage(sessionID, data, size);
}

void TcpServer::_RemoveConnection(uint64_t id) {
	m_sessions.erase(id);
}