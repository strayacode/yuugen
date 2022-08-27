#include "Core/ARM/Watchpoints.h"

Watchpoints::Watchpoints() {
    m_watchpoints.clear();
}

void Watchpoints::add(u32 addr) {
    Watchpoint watchpoint;

    // TODO: handle this correctly later
    watchpoint.type = WatchpointType::Execute;
    watchpoint.addr = addr;

    m_watchpoints.push_back(watchpoint);
}

bool Watchpoints::contains(u32 addr) {
    for (auto& watchpoint : m_watchpoints) {
        if (watchpoint.addr == addr) {
            return true;
        }
    }

    return false;
}