#pragma once

#include <unordered_map>
#include <google/protobuf/message.h>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "ClientInfo.h"
#include "Codec/Packet.h"

GAME_SERVICE_NS_BEGIN

class NetMgr : public INetwork, public IModule, public ISingleton<NetMgr> {
public:
	bool Init() override;
	bool UnInit() override;
    bool Update() override;
    bool Quit() override;

	bool OnNewSession(uint64_t clientID) override;
	bool OnCloseSession(uint64_t clientID) override;
	bool OnMessage(uint64_t clientID, const char* data, uint64_t len) override;

	//发送数据到客户端
	bool SendToClient(uint64_t clientID, int cmd, ::google::protobuf::Message* msg);

	bool CloseClient(uint64_t clientID);

	bool SetClientStatus(uint64_t clientID, bool isLogin);

private:
    bool _OnPackHandle(ClientInfo* info, const Packet* pack);
    bool _HandlePing(uint64_t clientID, const Packet* pack);

	std::unordered_map<uint64_t, ClientInfo*> m_clientInfos;
};

GAME_SERVICE_NS_END