#include <functional>
#include <vector>
#include <stdio.h>
#include <common/types.h>
#include <common/log.h>
#include <string>

using SchedulerCallback = std::function<void()>;

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
    void Reset();
    void Tick(int cycles);
    u64 GetCurrentTime();
    u64 GetEventTime();
    void ResetCurrentTime();
    void RunEvents();
    void AddEvent(u64 delay, EventType* type);
    void CancelEvent(EventType* type);
    int CalculateEventIndex(Event& event);

    EventType RegisterEvent(std::string name, SchedulerCallback callback);
private:
    u64 current_time;

    std::vector<Event> events;

    // this is increment on each register event call such that each event type is given
    // a unique id
    int current_event_id;
};