#pragma once

#include <util/types.h>
#include <util/log.h>
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

    // these are used when we read from ipcfiforecv
    u32 fifo7recv;
    u32 fifo9recv;

    void EmptyFIFO7();
    void EmptyFIFO9();

    // if the fifo is read from, we are reading from fiforecv
    // and if the fifo is written to, we are writing to fifosend
    u32 ReadFIFORECV7();
    u32 ReadFIFORECV9();

    void WriteFIFOSEND7(u32 data);
    void WriteFIFOSEND9(u32 data);

    Core* core;
};