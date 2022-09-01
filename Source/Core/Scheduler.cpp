#include "Common/Log.h"
#include "Core/Scheduler.h"

void Scheduler::reset() {
    m_events.clear();
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
    while (m_events.size() > 0 && m_events[0].time <= GetCurrentTime()) {
        m_events[0].type->callback();
        m_events.erase(m_events.begin());
    }
}

void Scheduler::AddEvent(u64 delay, EventType* type) {
    u64 time = GetCurrentTime() + delay;
    Event event{time, type};
    int index = CalculateEventIndex(event);

    m_events.insert(m_events.begin() + index, event);
}

void Scheduler::CancelEvent(EventType* type) {
    // TODO: assert that there can't be 2 events with the same id
    for (u64 i = 0; i < m_events.size(); i++) {
        if (m_events[i].type->id == type->id) {
            m_events.erase(m_events.begin() + i);
        }
    }
}

int Scheduler::CalculateEventIndex(Event& event) {
    int lower_bound = 0;
    int upper_bound = m_events.size() - 1;
   
    while (lower_bound <= upper_bound) {
        int mid = (lower_bound + upper_bound) / 2;

        if (event.time > m_events[mid].time) {
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
    return m_events[0].time;
}

void Scheduler::set_current_time(u64 data) {
    current_time = data;
}