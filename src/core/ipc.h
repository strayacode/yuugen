#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

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
    u32 IPCFIFOCNT7;
    u32 IPCFIFOCNT9;

    u32 fifo7[16];
    u32 fifo9[16];

    Core* core;
};