#pragma once

#include <queue>
#include <array>
#include "Common/Types.h"

class System;

class IPC {
public:
    IPC(System& system);

    void reset();
    void write_ipcsync(int cpu, u16 data);
    void write_ipcfifocnt(int cpu, u16 data);
    
    void empty_fifo(int cpu);

    u32 read_ipcfiforecv(int cpu);
    
    void write_send_fifo(int cpu, u32 data);

    union IPCSync {
        struct {
            u8 input : 4;
            u32 : 4;
            u8 output : 4;
            bool : 1;
            bool send_irq : 1;
            bool enable_irq : 1;
            u32 : 17;
        };

        u32 data;
    };

    union IPCFifoControl {
        struct {
            bool send_fifo_empty : 1;
            bool send_fifo_full : 1;
            bool send_fifo_empty_irq : 1;
            bool send_fifo_clear : 1;
            u32 : 4;
            bool receive_fifo_empty : 1;
            bool receive_fifo_full : 1;
            bool receive_fifo_empty_irq : 1;
            u32 : 3;
            bool error : 1;
            bool enable_fifos : 1;
        };

        u16 data;
    };

    std::array<IPCSync, 2> ipcsync;
    std::array<IPCFifoControl, 2> ipcfifocnt;
    std::array<std::queue<u32>, 2> fifo;
    std::array<u32, 2> ipcfiforecv;

    System& system;
};