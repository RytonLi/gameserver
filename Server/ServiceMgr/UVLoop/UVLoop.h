#pragma once

#include <functional>

#include "Utils/Define.h"
#include "UVInclude/uv.h"

MY_SERVICE_MGR_NS_BEGIN

using DefaultCallback = std::function<void()>;

class UVLoop {
public:
	bool Init();
	bool UnInit();

	bool SetFrameRate(uint8_t frameRate);
	bool SetUpdateCallback(DefaultCallback callback);

	bool Start();

    uv_loop_t* Loop();

    uint64_t Now();

private:
	void _UpdateTime();

	uv_loop_t*      m_loop;
	uint8_t         m_frameRate;
	DefaultCallback m_updateCallback;
};

MY_SERVICE_MGR_NS_END