#include "Common/Log.h"
#include "Core/scheduler/scheduler.h"

void Scheduler::Reset() {
    events.clear();
    current_time = 0;
    current_event_id = 0;
}

void Scheduler::Tick(int cycles) {
    current_time += cycles;
}

void Scheduler::ResetCurrentTime() {
    current_time = 0;
}

void Scheduler::RunEvents() {
    while (events.size() > 0 && events[0].time <= GetCurrentTime()) {
        events[0].type->callback();
        events.erase(events.begin());
    }
}

void Scheduler::AddEvent(u64 delay, EventType* type) {
    u64 time = GetCurrentTime() + delay;
    Event event{time, type};
    int index = CalculateEventIndex(event);
    events.insert(events.begin() + index, event);
}

void Scheduler::CancelEvent(EventType* type) {
    // TODO: assert that there can't be 2 events with the same id
    for (u64 i = 0; i < events.size(); i++) {
        if (events[i].type->id == type->id) {
            events.erase(events.begin() + i);
        }
    }
}

int Scheduler::CalculateEventIndex(Event& event) {
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

EventType Scheduler::RegisterEvent(std::string name, SchedulerCallback callback) {
    EventType type;
    type.name = name;
    type.id = current_event_id;
    type.callback = callback;
    current_event_id++;
    
    return type;
}

u64 Scheduler::GetCurrentTime() const {
    return current_time;
}

u64 Scheduler::GetEventTime() const {
    return events[0].time;
}

std::vector<Event>& Scheduler::GetEvents() {
    return events;
}