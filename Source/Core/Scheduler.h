#pragma once

#include <vector>
#include <string>
#include "Common/Types.h"
#include "Common/Callback.h"

using SchedulerCallback = Common::Callback<void()>;

struct EventType {
    std::string name;
    int id;
    SchedulerCallback callback;
};

struct Event {
    u64 time;
    EventType* type;
};

class Scheduler {
public:
    void reset();
    void Tick(int cycles);
    void ResetCurrentTime();
    void RunEvents();
    void AddEvent(u64 delay, EventType* type);
    void CancelEvent(EventType* type);
    int CalculateEventIndex(Event& event);
    EventType RegisterEvent(std::string name, SchedulerCallback callback);

    u64 GetCurrentTime() const;
    u64 GetEventTime() const;
    void set_current_time(u64 data);

    std::vector<Event>& events() { return m_events; }
    
private:
    std::vector<Event> m_events;
    u64 current_time;
    int current_event_id;
};