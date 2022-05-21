#include "Core/HW/ipc/ipc.h"
#include "Core/core.h"

IPC::IPC(System& system) : system(system) {}

void IPC::reset() {
    ipcsync[0].data = 0;
    ipcsync[1].data = 0;
    ipcfifocnt[0].data = 0x101;
    ipcfifocnt[1].data = 0x101;
    ipcfiforecv[0] = 0;
    ipcfiforecv[1] = 0;

    empty_fifo(0);
    empty_fifo(1);
}

void IPC::write_ipcsync(int cpu, u16 data) {
    int remote = !cpu;

    ipcsync[cpu].data = (ipcsync[cpu].data & ~0x6F00) | (data & 0x6F00);
    ipcsync[remote].input = ipcsync[cpu].output;
    
    if (ipcsync[cpu].send_irq && ipcsync[remote].enable_irq) {
        system.cpu_core[cpu].SendInterrupt(InterruptType::IPCSync);
    }
}

void IPC::write_ipcfifocnt(int cpu, u16 data) {
    int remote = !cpu;
    bool old_send_fifo_empty_irq = ipcfifocnt[cpu].send_fifo_empty_irq;
    bool old_receive_fifo_empty_irq = ipcfifocnt[cpu].receive_fifo_empty_irq;

    ipcfifocnt[cpu].data = (ipcfifocnt[cpu].data & ~0x8404) | (data & 0x8404);

    // empty the send fifo
    if (data & (1 << 3)) {
        empty_fifo(cpu);
        
        ipcfifocnt[cpu].send_fifo_empty = true;
        ipcfifocnt[cpu].send_fifo_full = false;
        ipcfifocnt[remote].receive_fifo_empty = true;
        ipcfifocnt[remote].receive_fifo_full = false;

        if (ipcfifocnt[cpu].send_fifo_empty_irq) {
            system.cpu_core[cpu].SendInterrupt(InterruptType::IPCSendEmpty);
        }
    }

    if (!old_send_fifo_empty_irq && ipcfifocnt[cpu].send_fifo_empty_irq && ipcfifocnt[cpu].send_fifo_empty) {
        system.cpu_core[cpu].SendInterrupt(InterruptType::IPCSendEmpty);
    }

    if (!old_receive_fifo_empty_irq && ipcfifocnt[cpu].receive_fifo_empty_irq && ipcfifocnt[cpu].receive_fifo_empty) {
        system.cpu_core[cpu].SendInterrupt(InterruptType::IPCReceiveNonEmpty);
    }

    if (data & (1 << 14)) {
        ipcfifocnt[cpu].error = false;
    }
}

void IPC::empty_fifo(int cpu) {
    std::queue<u32> empty_queue;
    fifo[cpu].swap(empty_queue);
}

u32 IPC::read_ipcfiforecv(int cpu) {
    int remote = !cpu;

    if (!fifo[remote].empty()) {
        ipcfiforecv[cpu] = fifo[remote].front();

        if (ipcfifocnt[cpu].enable_fifos) {
            fifo[remote].pop();

            if (fifo[remote].empty()) {
                ipcfifocnt[remote].send_fifo_empty = true;
                ipcfifocnt[cpu].receive_fifo_empty = true;
                
                if (ipcfifocnt[remote].send_fifo_empty_irq) {
                    system.cpu_core[remote].SendInterrupt(InterruptType::IPCSendEmpty);
                }
            } else if (fifo[remote].size() == 15) {
                ipcfifocnt[remote].send_fifo_full = false;
                ipcfifocnt[cpu].receive_fifo_full = false;
            }
        }
    } else {
        ipcfifocnt[cpu].error = true;
    }

    return ipcfiforecv[cpu];
}

void IPC::write_send_fifo(int cpu, u32 data) {
    int remote = !cpu;

    if (ipcfifocnt[cpu].enable_fifos) {
        if (fifo[cpu].size() < 16) {
            fifo[cpu].push(data);

            if (fifo[cpu].size() == 1) {
                ipcfifocnt[cpu].send_fifo_empty = false;
                ipcfifocnt[remote].receive_fifo_empty = false;

                if (ipcfifocnt[remote].receive_fifo_empty_irq) {
                    system.cpu_core[remote].SendInterrupt(InterruptType::IPCReceiveNonEmpty);
                }
            } else if (fifo[cpu].size() == 16) {
                ipcfifocnt[cpu].send_fifo_full = true;
                ipcfifocnt[remote].receive_fifo_full = true;
            }
        } else {
            ipcfifocnt[cpu].error = true;
        }
    }
}