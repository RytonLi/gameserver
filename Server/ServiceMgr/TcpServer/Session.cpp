#include "Session.h"
#include "Log/Log.h"

using namespace std;
using namespace msm;

struct WriteReq {
	uv_write_t req;
	uv_buf_t buf;
	AfterWriteCallback callback;
	void* data;
};

WriteReq* _NewWriteReq(ssize_t size) {
	WriteReq* wr = nullptr;
	char* tmp = nullptr;

	tmp = (char*)malloc(size);
	if (!tmp) {
		goto Exit0;
	}

	wr = new WriteReq;

	wr->buf = uv_buf_init(tmp, size);

Exit0:
	return wr;
}

void _DeleteWriteReq(WriteReq* wr) {
	if (wr != nullptr) {
		free(wr->buf.base);
		delete wr;
	}
}

bool Session::Init(uint64_t id, uv_loop_t* loop) {
	m_id          = id;
	m_loop        = loop;
	m_client.data = this;
	m_isConnected = false;

	m_onMessageCallback      = nullptr;
	m_onConnectCloseCallback = nullptr;
	m_closeCompleteCallback  = nullptr;

	::uv_tcp_init(m_loop, &m_client);
	return true;
}

bool Session::Start() {
	bool ret = false;
	if (::uv_read_start((uv_stream_t*)&m_client,
		//匿名函数，缓冲分配
		[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
			buf->base = (char*)malloc(suggested_size);
			buf->len = (int)suggested_size;
		}
		, _OnMesageReceive) == 0) {
		ret = true;
		m_isConnected = true;
	}
	return ret;
}

//回调onConnectCloseCallback
void Session::OnSessionClose() {
	//自己不主动close，通知上层close
    if (m_onConnectCloseCallback) {
        m_onConnectCloseCallback(m_id);
    }
}

//关闭uvClient
void Session::Close(SessionCallback callback) {
    m_isConnected = false;
	m_onMessageCallback      = nullptr;
	m_onConnectCloseCallback = nullptr;
	m_closeCompleteCallback  = nullptr;

	m_closeCompleteCallback = callback;
	uv_tcp_t* ptr = &m_client;
	if (::uv_is_active((uv_handle_t*)ptr)) {
		::uv_read_stop((uv_stream_t*)ptr);
	}

	if (::uv_is_closing((uv_handle_t*)ptr) == 0) {
		::uv_close((uv_handle_t*)ptr,
			//匿名函数，close完成后回调closeCompleteCallback
			[](uv_handle_t* handle) {
				Session* conn = (Session*)handle->data;
				conn->_CloseComplete();
			});
	}
}

bool Session::Send(const char* buf, ssize_t size, AfterWriteCallback callback, void* data) {
	bool      ret = false;
	WriteReq* req = nullptr;
	int       res = -1;

    //连接已断开
	if (!m_isConnected) {
        LOG_ERROR("send fail, client: %lld is disconnected", m_id);
        goto Exit0;
	}

    if (!::uv_is_writable((uv_stream_t*)&m_client)) {
        LOG_ERROR("send fail, client: %lld isn't writable", m_id);
        goto Exit0;
    }

	req = _NewWriteReq(size);
	if (!req) {
		LOG_ERROR("send fail, client: %lld new write req fail", m_id);
		goto Exit0;
	}

	memcpy(req->buf.base, buf, size);

	req->callback = callback;
	req->data     = data;
    res = ::uv_write((uv_write_t*)req, (uv_stream_t*)&m_client, &req->buf, 1,
		//匿名函数，若设置了AfterWriteCallback则调用
		[](uv_write_t *req, int status) {
            if (status < 0) {
                LOG_ERROR("send fail, uv error: %s", ::uv_strerror(status));
            }

			WriteReq* wr = (WriteReq*)req;
			if (nullptr != wr->callback) {
				wr->callback(status, wr->data);
			}
			_DeleteWriteReq(wr);
		});

	if (0 != res) {
		LOG_ERROR("send fail, client: %lld uv_write fail", m_id);
		goto Exit0;
	}

	req = nullptr;
	ret = true;
Exit0:
	if (req) {
		_DeleteWriteReq(req);
	}
	return ret;
}

void Session::SetMessageCallback(OnMessageCallback callback) {
	m_onMessageCallback = callback;
}

void Session::SetConnectCloseCallback(SessionCallback callback) {
	m_onConnectCloseCallback = callback;
}

uv_tcp_t* Session::Handle() {
	return &m_client;
}

//uv_read_start回调
void Session::_OnMesageReceive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
	Session* conn = (Session*)client->data;
    uint64_t id   = conn->m_id;
	//有数据写入
	if (nread > 0) {
		//回调
		conn->_OnMessage(buf->base, nread);
		goto Exit0;
	}

	//出错
	if (nread < 0) {
		//设置连接断开
		conn->_SetConnectStatus(false);

		//如果不是EOF
		if (nread != UV_EOF) {
			LOG_ERROR("on msg receive, client: %lld error", id);
			conn->OnSessionClose();
			goto Exit0;
		}

		//EOF，先shutdown
        LOG_ERROR("on msg receive, client: %lld disconnect", id);
        uv_shutdown_t* req = new uv_shutdown_t;
		req->data = conn;
		::uv_shutdown(req, (uv_stream_t*)client,
			//匿名函数，shutdown完成后再调用OnSocketClose
			[](uv_shutdown_t* req, int status) {
				Session* conn = (Session*)req->data;
				conn->OnSessionClose();
				delete req;
			});
	}

Exit0:
	if (buf->base) {
		free(buf->base);
	}
	return;
}

void Session::_SetConnectStatus(bool isConnected) {
	m_isConnected = isConnected;
}

void Session::_OnMessage(const char* buf, ssize_t size) {
	if (m_onMessageCallback) {
		m_onMessageCallback(m_id, buf, size);
	}
}

void Session::_CloseComplete() {
	if (m_closeCompleteCallback) {
		m_closeCompleteCallback(m_id);
	}
}