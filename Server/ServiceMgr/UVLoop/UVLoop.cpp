#include "UVLoop.h"
#include "Log/Log.h"

using namespace msm;

bool UVLoop::Init() {
	m_loop           = uv_default_loop();
	m_frameRate	     = 30;
	m_updateCallback = nullptr;
	return true;
}

bool UVLoop::UnInit() {
	return true;
}

bool UVLoop::SetFrameRate(uint8_t frameRate) {
	m_frameRate = frameRate;
    return true;
}

bool UVLoop::SetUpdateCallback(DefaultCallback callback) {
	m_updateCallback = callback;
    return true;
}

bool UVLoop::Start() {
	_UpdateTime();

	const double frameInterval = (double)1000 / m_frameRate; //帧间隔

	uint64_t start  = Now(); //开始时间戳
	uint64_t ticks  = 0; //帧数
	int      wait   = 0; //距下帧等待时间

	for (;;) {
        //LOG_DEBUG("ticks: %d", ticks);
		if (m_updateCallback) {
			m_updateCallback();
		}

		ticks++;

        _UpdateTime();

        uv_run(m_loop, UV_RUN_NOWAIT);
		
		wait = frameInterval * ticks - (Now() - start);
		if (wait < 0) {
            //LOG_DEBUG("frame work timeout");
			continue;
		}
		else {
            //usleep(wait * 1000);
            Sleep(wait);
		}
	}

	return true;
}

uv_loop_t* UVLoop::Loop() {
    return m_loop;
}

uint64_t UVLoop::Now() {
    return uv_now(m_loop);
}

void UVLoop::_UpdateTime() {
	uv_update_time(m_loop);
}

