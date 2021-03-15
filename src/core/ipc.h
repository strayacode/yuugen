#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <queue>

struct Core;

struct IPC {
    IPC(Core* core);
    void Reset();

    void WriteIPCSYNC7(u16 data);
    void WriteIPCSYNC9(u16 data);
    u16 ReadIPCSYNC7();

    u16 ReadIPCSYNC9();


    void WriteIPCFIFOCNT7(u16 data);
    void WriteIPCFIFOCNT9(u16 data);
    


    u16 IPCSYNC7;
    u16 IPCSYNC9;
    u16 IPCFIFOCNT7;
    u16 IPCFIFOCNT9;

    std::queue<u32> fifo7;
    std::queue<u32> fifo9;

    void EmptyFIFO7();
    void EmptyFIFO9();

    Core* core;
};