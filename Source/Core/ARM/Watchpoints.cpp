#include "Core/ARM/Watchpoints.h"

Watchpoints::Watchpoints() {
    m_watchpoints.clear();
}

void Watchpoints::add(u32 addr) {
    Watchpoint watchpoint;

    // TODO: handle this correctly later
    watchpoint.type = WatchpointType::Execute;
    watchpoint.addr = addr;
    watchpoint.enabled = true;
    watchpoint.auto_remove = true;

    m_watchpoints.push_back(watchpoint);
}

bool Watchpoints::contains(u32 addr) {
    for (u64 i = 0; i < m_watchpoints.size(); i++) {
        auto& watchpoint = m_watchpoints[i];
        if (watchpoint.enabled && watchpoint.addr == addr) {
            if (watchpoint.auto_remove) {
                // remove on watchpoint being hit
                m_watchpoints.erase(m_watchpoints.begin() + i);
            }

            return true;
        }
    }

    return false;
}