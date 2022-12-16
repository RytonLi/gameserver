#pragma once

#include <unordered_map>
#include <functional>

#include "../Utils/Define.h"
#include "../UVInclude/uv.h"

namespace msm {

using TimeoutCallback = std::function<void()>;

class UVTimerMgr {
public:
	UVTimerMgr() = default;
	virtual ~UVTimerMgr() = default;

	bool Init(uv_loop_t* loop);
	bool UnInit();

	//���Ӷ�ʱ�������ض�ʱ��ID
	int AddTimer(uint64_t timeout, bool repeat, TimeoutCallback callback);

	//�Ƴ���ʱ��
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

}