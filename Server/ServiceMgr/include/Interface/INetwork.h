#pragma once

#include <stdint.h>
#include <stddef.h>

class INetwork {
public:
	virtual bool OnNewSession(uint64_t sessionID) = 0;
    virtual bool OnCloseSession(uint64_t sessionID) = 0;
	virtual bool OnMessage(uint64_t sessionID, const char* data, uint64_t len) = 0;
};