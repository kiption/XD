#pragma once
#include <chrono>
#include <queue>
#include <mutex>
using namespace std;
using namespace chrono;

enum TimerObjType { OBJ_BULLET };
enum EventType { EV_MOVE };
struct TimerEvent {
	int obj_type;
	int obj_id;
	EventType ev_type;
	system_clock::time_point act_time;
	
	constexpr bool operator< (const TimerEvent& other_ev) const {
		return (act_time > other_ev.act_time);
	}
};

priority_queue<TimerEvent> timer_queue;
mutex timer_lock;

void addEvent(int object_type, int object_id, EventType event_type, int time_count_ms)
{
	TimerEvent ev;
	ev.obj_type = object_type;
	ev.obj_id = object_id;
	ev.ev_type = event_type;
	ev.act_time = system_clock::now() + milliseconds(time_count_ms);
	timer_queue.push(ev);
}