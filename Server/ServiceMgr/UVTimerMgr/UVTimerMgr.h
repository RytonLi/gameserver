#pragma once

#include <unordered_map>
#include <functional>

#include "Utils/Define.h"
#include "UVInclude/uv.h"

MY_SERVICE_MGR_NS_BEGIN

using TimeoutCallback = std::function<void()>;

class UVTimerMgr {
public:
	UVTimerMgr() = default;
	virtual ~UVTimerMgr() = default;

	bool Init(uv_loop_t* loop);
	bool UnInit();

	//添加定时器，返回定时器ID
	int AddTimer(uint64_t timeout, bool repeat, TimeoutCallback callback);

	//移除定时器
	bool RemoveTimer(int timerID);

private:
	struct TimerMsg {
		uv_timer_t      uvTimer;
		UVTimerMgr*        thisPtr;
		int             timerID;
		TimeoutCallback callback;

		TimerMsg(UVTimerMgr* thisPtr, int timerID, TimeoutCallback callback) :
			thisPtr(thisPtr), 
			timerID(timerID), 
			callback(callback) {}
	};

private:
	static void _OnTimer(uv_timer_t* timer);
	void _CloseTimer(int timerID);

	uv_loop_t* m_loop;
	int        m_autoIncID;

	std::unordered_map<int, TimerMsg*> m_timerMap;
};

MY_SERVICE_MGR_NS_END