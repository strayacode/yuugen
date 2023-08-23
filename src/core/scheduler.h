#pragma once

#include <vector>
#include <string>
#include "common/types.h"
#include "common/callback.h"

namespace core {

using SchedulerCallback = common::Callback<void()>;

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
    void tick(int cycles);
    void run();
    void add_event(u64 delay, EventType* type);
    void cancel_event(EventType* type);
    EventType register_event(std::string name, SchedulerCallback callback);

    u64 get_current_time();
    u64 get_event_time();
    void set_current_time(u64 value);

    std::vector<Event>& get_events() { return events; }
    
private:
    int calculate_event_index(Event& event);

    std::vector<Event> events;
    u64 current_time;
    int current_event_id;
};

} // namespace core