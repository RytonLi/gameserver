#pragma once

#include <unordered_map>

#include "Session.h"
#include "../Interface/INetwork.h"

namespace msm {

using DefaultCallback            = std::function<void()>;
using OnConnectionStatusCallback = std::function<void(uint64_t)>;

class TcpServer {
public:
	TcpServer() = default;
	~TcpServer() = default;

	//��ʼ��
	bool Init(uv_loop_t* loop, INetwork* network);
	bool UnInit();
	
	//��socket��ʼ����
	bool BindAndListen(const char* ip, int port);
	//�ر�serverSocket���Լ���������
	void Close(DefaultCallback callback);

	Session* GetSession(uint64_t sessionID);
	//�ر�һ�����ӣ���������/connection onclose�Ļص�
	void CloseSession(uint64_t sessionID);
	//�ر���������
	void CloseAllSession();

	//�����ӷ������ݣ�������һ�����εĻص�
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

}