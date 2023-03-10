#pragma once

#include <string.h>
#include <functional>

#include "UVInclude/uv.h"
#include "Utils/Define.h"

MY_SERVICE_MGR_NS_BEGIN

using SessionCallback    = std::function<void(uint64_t)>;
using OnMessageCallback  = std::function<void(uint64_t, const char*, ssize_t)>;
using AfterWriteCallback = std::function<void(uint64_t, void*)>;

class Session {
public:
    Session() = default;
	virtual ~Session() = default;

	//初始化
	bool Init(uint64_t id, uv_loop_t* loop);
	//uv_read_start
	bool Start();

	//回调onConnectCloseCallback
	void OnSessionClose();
	//关闭uvClient
	void Close(SessionCallback callback);

	//发送数据
	bool Send(const char* buf, ssize_t size, AfterWriteCallback callback, void* data);

	void SetMessageCallback(OnMessageCallback callback);
	void SetConnectCloseCallback(SessionCallback callback);

	uv_tcp_t* Handle();
private:
	//uv_read_start回调
	static void _OnMesageReceive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);

	void _SetConnectStatus(bool isConnected);
	void _OnMessage(const char* buf, ssize_t size);
	void _CloseComplete();

    uint64_t   m_id;
	uv_loop_t* m_loop;
	uv_tcp_t   m_client;
	bool       m_isConnected;

	OnMessageCallback m_onMessageCallback;
    SessionCallback   m_onConnectCloseCallback;
    SessionCallback   m_closeCompleteCallback;
};

MY_SERVICE_MGR_NS_END