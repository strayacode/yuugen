#include "common/logger.h"
#include "common/scheduler.h"

namespace common {

void Scheduler::reset() {
    events.clear();
    current_time = 0;
    current_event_id = 0;
}

void Scheduler::tick(int cycles) {
    current_time += cycles;
}

void Scheduler::run() {
    while (events.size() > 0 && events[0].time <= current_time) {
        events[0].type->callback();
        events.erase(events.begin());
    }
}

void Scheduler::add_event(u64 delay, EventType* type) {
    u64 time = current_time + delay;
    Event event{time, type};
    auto index = calculate_event_index(event);
    events.insert(events.begin() + index, event);
}

void Scheduler::cancel_event(EventType* type) {
    // TODO: assert that there can't be 2 events with the same id
    for (u64 i = 0; i < events.size(); i++) {
        if (events[i].type->id == type->id) {
            events.erase(events.begin() + i);
        }
    }
}

EventType Scheduler::register_event(std::string name, SchedulerCallback callback) {
    EventType type;
    type.name = name;
    type.id = current_event_id;
    type.callback = callback;
    current_event_id++;
    return type;
}

u64 Scheduler::get_current_time() {
    return current_time;
}

u64 Scheduler::get_event_time() {
    return events[0].time;
}

void Scheduler::set_current_time(u64 value) {
    current_time = value;
}

int Scheduler::calculate_event_index(Event& event) {
    if (events.size() == 0) {
        return 0;
    }

    int lower_bound = 0;
    int upper_bound = events.size() - 1;

    while (lower_bound <= upper_bound) {
        int mid = (lower_bound + upper_bound) / 2;
        if (event.time > events[mid].time) {
            lower_bound = mid + 1;
        } else {
            upper_bound = mid - 1;
        }
    }

    return lower_bound;
}

} // namespace common