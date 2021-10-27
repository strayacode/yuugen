#pragma once

#include <queue>
#include <common/types.h>

class System;

class IPC {
public:
    IPC(System& system);
    void Reset();

    void WriteIPCSYNC7(u16 data);
    void WriteIPCSYNC9(u16 data);
    auto ReadIPCSYNC7() -> u16;

    auto ReadIPCSYNC9() -> u16;

    void WriteIPCFIFOCNT7(u16 data);
    void WriteIPCFIFOCNT9(u16 data);

    void EmptyFIFO7();
    void EmptyFIFO9();

    // if the fifo is read from, we are reading from fiforecv
    // and if the fifo is written to, we are writing to fifosend
    auto ReadFIFORECV7() -> u32;
    auto ReadFIFORECV9() -> u32;

    void WriteFIFOSEND7(u32 data);
    void WriteFIFOSEND9(u32 data);
    
    u16 IPCSYNC7;
    u16 IPCSYNC9;
    u16 IPCFIFOCNT7;
    u16 IPCFIFOCNT9;

    std::queue<u32> fifo7;
    std::queue<u32> fifo9;

    // these are used when we read from ipcfiforecv
    u32 fifo7recv;
    u32 fifo9recv;

    System& system;
};