#include "NetMgr.h"
#include "Proto/player.pb.h"
#include "Codec/Codec.h"
#include "NetPackDispatcher/NetPackDispatcher.h"

#define CHECK_TIMEOUT_RATE 10
#define CLIENT_TIMEOUT 60

using namespace std;
using namespace gs;
using namespace TCCamp;

extern msm::Service* g_gameService;

static char gs_sendBuffer[64 * 1024];

bool NetMgr::Init() {
	m_clientInfos.clear();

    //超时检测
    msm::ServiceMgr::Ins().AddTimer(
        CHECK_TIMEOUT_RATE * 1000, 
        true,
        [this]() {
            for (auto it = m_clientInfos.begin(); it != m_clientInfos.end(); it++) {
                if (msm::ServiceMgr::Ins().Now() >= it->second->expireTime) {
                    CloseClient(it->second->clientID);
                }
            }
        }
    );

	return true;
}

bool NetMgr::UnInit() {
	return true;
}

bool NetMgr::Update() {
    return true;
}

bool NetMgr::Quit() {
    return true;
}

//新连接建立
bool NetMgr::OnNewSession(uint64_t clientID) {
	bool res = false;

	ClientInfo* info = NewClientInfo(clientID);
	if (!info) {
		LOG_ERROR("new client: %lld fail, reason: new info fail", clientID);
		goto Exit0;
	}
	LOG_DEBUG("new client: %lld", clientID);

    info->expireTime = msm::ServiceMgr::Ins().Now() + CLIENT_TIMEOUT * 1000;

	m_clientInfos.insert(pair<uint64_t, ClientInfo*>(clientID, info));

	res = true;
Exit0:
    return res;
}

//有连接关闭
bool NetMgr::OnCloseSession(uint64_t clientID) {
	ClientInfo* info = nullptr;

	auto it = m_clientInfos.find(clientID);
	if (it == m_clientInfos.end()) {
		LOG_ERROR("on close client error, client: %lld not found", clientID);
		goto Exit0;
	}
	LOG_DEBUG("close client: %lld", clientID);
	info = it->second;

    NetPackDispatcher::Ins().DispatchLogout(clientID);

	m_clientInfos.erase(it);
	DeleteClientInfo(info);

Exit0:
	return true;
}

bool NetMgr::OnMessage(uint64_t clientID, const char* data, uint64_t len) {
	bool result = false;
	ClientInfo* info = nullptr;
	PeerData* peerData = nullptr;
	int start;

	auto it = m_clientInfos.find(clientID);
	if (it == m_clientInfos.end()) {
		LOG_ERROR("on recv client data error, client: %lld not found", clientID);
		goto Exit0;
	}

	info = it->second;
	peerData = &(info->peerData);

	if (sizeof(peerData->buff) - peerData->now < len) {
		LOG_ERROR("on recv client data error, client: %lld recv buff full", clientID);
		goto Exit0;
	}

	memcpy(peerData->buff + peerData->now, data, len);
	peerData->now += len;

	start = 0;

	// 完整协议包检测，处理网络粘包
	while (true) {
		int packPos = check_pack(peerData->buff + start, peerData->now - start);
		if (packPos < 0) {          // 异常
			LOG_ERROR("on recv client data error, client: %lld check pack fail", clientID);
			goto Exit0;
		}
		else if (packPos > 0) {     // 协议数据完整了
			Packet pack;        // 协议包格式
			// 解码，得到协议号和序列化后的数据
			if (!decode(&pack, peerData->buff + start, packPos)) {
				goto Exit0;
			}

			// 处理一个数据包
            if (!_OnPackHandle(info, &pack)) {
                goto Exit0;
            }

			start += packPos;
		}
		else {
			// 没有完全到达完整包
			memmove(peerData->buff, peerData->buff + start, peerData->now - start);
			peerData->now -= start;
			break;
		}
	}

	result = true;
Exit0:
	if (!result) {
		CloseClient(clientID);
	}
	return true;
}

bool NetMgr::SendToClient(uint64_t clientID, int cmd, ::google::protobuf::Message* msg) {
	bool ret = false;
	auto it = m_clientInfos.find(clientID);
	string data;
	int len;

	if (it == m_clientInfos.end()) {
		LOG_ERROR("on send to client data error, client: %lld not found", clientID);
		goto Exit0;
	}

	if (msg != nullptr) {
		data = msg->SerializeAsString();
	}
	len = encode(gs_sendBuffer, cmd, data.c_str(), data.length());
	ret = g_gameService->Send(clientID, gs_sendBuffer, len);

    //if (len > 1024) {
    //    LOG_DEBUG("on send to client data, client: %d, data len: %d\n", clientID, len);
    //}

Exit0:
	return ret;
}

bool NetMgr::CloseClient(uint64_t clientID) {
	auto it = m_clientInfos.find(clientID);
	if (it != m_clientInfos.end()) {
        g_gameService->Close(clientID);
	}
	else {
		LOG_ERROR("on close client error, client: %d not found", clientID);
	}
	return true;
}

bool NetMgr::SetClientStatus(uint64_t clientID, bool isLogin) {
	bool ret = false;
	auto it = m_clientInfos.find(clientID);
	if (it == m_clientInfos.end()) {
		goto Exit0;
	}

	it->second->isLogin = isLogin;

	ret = true;
Exit0:
	return ret;
}

bool NetMgr::_OnPackHandle(ClientInfo* info, const Packet* pack) {
    bool ret = false;
    info->expireTime = msm::ServiceMgr::Ins().Now() + CLIENT_TIMEOUT * 1000;
    if (pack->cmd == CLIENT_CMD::CLIENT_PING) {
        ret = _HandlePing(info->clientID, pack);
    }
    else if (info->isLogin) {
        //LOG_DEBUG("on client: %lld, pack cmd: %d", info->clientID, pack->cmd);
        ret = NetPackDispatcher::Ins().Dispatch(pack->cmd, info->clientID, pack);
    }
    else if (pack->cmd == CLIENT_CMD::CLIENT_LOGIN_REQ ||
        pack->cmd == CLIENT_CMD::CLIENT_CREATE_REQ) {
        ret = NetPackDispatcher::Ins().Dispatch(pack->cmd, info->clientID, pack);
    }
    else {
        LOG_DEBUG("unexpected cmd: %d", pack->cmd);
    }

    return ret;
}

bool NetMgr::_HandlePing(uint64_t clientID, const Packet* pack) {
    //LOG_DEBUG("client: %lld ping", clientID);
    ClientPingReq req;
    ServerPongRsp rsp;

    if (pack) {
        req.ParseFromArray(pack->data, pack->len);
        rsp.set_result(PB_RESULT_CODE::SUCCESS);
        rsp.set_timestamp(req.timestamp());
    }

    SendToClient(clientID, SERVER_CMD::SERVER_PONG, &rsp);
    return true;
}