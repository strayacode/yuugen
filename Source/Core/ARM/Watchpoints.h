#pragma once

#include <vector>
#include "Common/Types.h"

enum class WatchpointType {
    Read,
    Write,
    Execute,
};

struct Watchpoint {
    WatchpointType type;
    u32 addr;
};

// an abstraction for handling watchpoints for the arm7 and arm9
class Watchpoints {
public:
    Watchpoints();

    void add(u32 addr);
    bool contains(u32 addr);

    std::vector<Watchpoint>& get() { return m_watchpoints; }
private:
    static constexpr int NUM_BITS = 16;
    static constexpr int TABLE_SIZE = 1 << NUM_BITS;
    
    std::vector<Watchpoint> m_watchpoints;
};