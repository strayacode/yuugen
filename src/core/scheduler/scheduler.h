#include <functional>
#include <vector>
#include <stdio.h>
#include <common/types.h>
#include <common/log.h>

struct Event {
    u64 start_time;
    int id;
    std::function<void()> callback;
};

struct Scheduler {
    void Reset();
    void Tick(int cycles);
    auto GetCurrentTime() -> u64;
    void ResetCurrentTime();
    void Step();
    void Add(u64 delay, std::function<void()> callback);
    auto CalculateEventIndex(Event& new_event) -> int;

    u64 current_time;

    std::vector<Event> events;
};