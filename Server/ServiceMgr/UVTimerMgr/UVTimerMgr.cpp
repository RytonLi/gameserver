#include "UVTimerMgr.h"

using namespace msm;
using namespace std;

bool UVTimerMgr::Init(uv_loop_t* loop) {
	m_loop      = loop;
	m_autoIncID = 1;
	m_timerMap.clear();
	return true;
}

bool UVTimerMgr::UnInit() {
	return true;
}

int UVTimerMgr::AddTimer(uint64_t timeout, bool repeat, TimeoutCallback callback) {
	int timerID = m_autoIncID++;
	TimerMsg* msg = new TimerMsg(this, timerID, callback);

	::uv_timer_init(m_loop, (uv_timer_t*)msg);
	m_timerMap.insert(pair<int, TimerMsg*>(timerID, msg));
	::uv_timer_start((uv_timer_t*)msg, _OnTimer, timeout, repeat ? timeout : 0);
	
	return timerID;
}

bool UVTimerMgr::RemoveTimer(int timerID) {
	_CloseTimer(timerID);
	return true;
}

void UVTimerMgr::_OnTimer(uv_timer_t* timer) {
	TimerMsg* msg = (TimerMsg*)timer;

	msg->callback();

	if (!timer->repeat) {
		msg->thisPtr->_CloseTimer(msg->timerID);
	}
}

void UVTimerMgr::_CloseTimer(int timerID) {
	TimerMsg* msg = nullptr;
	auto it = m_timerMap.find(timerID);
	if (it == m_timerMap.end()) {
		return;
	}

	msg = it->second;
	m_timerMap.erase(it);

	if (::uv_is_active((uv_handle_t*)msg)) {
		::uv_timer_stop((uv_timer_t*)msg);
	}
	
	if (::uv_is_closing((uv_handle_t*)msg) == 0) {
		::uv_close((uv_handle_t*)msg,
			[](uv_handle_t* handle) { delete handle; });
	}
}