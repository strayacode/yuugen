#include <core/ipc.h>
#include <core/core.h>

IPC::IPC(Core* core) : core(core) {

}

void IPC::Reset() {
    IPCSYNC7 = 0;
    IPCSYNC9 = 0;
    IPCFIFOCNT7 = 0x101;
    IPCFIFOCNT9 = 0x101;
}

void IPC::WriteIPCSYNC7(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC7 = (IPCSYNC7 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync7 (bits 8..11) to bits 0..3 of ipcsync9
    IPCSYNC9 = (IPCSYNC9 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    // printf("counter: %d\n", core->arm7.counter);
    // printf("write ipcsync7\n");
    // printf("ipcsync7: %08x ipcsync9: %08x\n", IPCSYNC7, IPCSYNC9);
    // if ipcsync 7 is sending an interrupt and interrupts are enabled on ipcsync9
    if ((IPCSYNC7 & (1 << 13)) && (IPCSYNC9 & (1 << 14))) {
        core->arm9.SendInterrupt(16);
    }
}


void IPC::WriteIPCSYNC9(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC9 = (IPCSYNC9 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync9 (bits 8..11) to bits 0..3 of ipcsync7
    IPCSYNC7 = (IPCSYNC7 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    // printf("counter: %d opcode: %08x\n", core->arm9.counter, core->arm9.instruction);
    // printf("write ipcsync9\n");
    // printf("ipcsync7: %08x ipcsync9: %08x\n", IPCSYNC7, IPCSYNC9);
    // if ipcsync 9 is sending an interrupt and interrupts are enabled on ipcsync 7
    if ((IPCSYNC9 & (1 << 13)) && (IPCSYNC7 & (1 << 14))) {
        core->arm7.SendInterrupt(16);
    }
}

u16 IPC::ReadIPCSYNC7() {
    return IPCSYNC7 & 0x4F0F;
}

u16 IPC::ReadIPCSYNC9() {
    return IPCSYNC9 & 0x4F0F;
}

void IPC::WriteIPCFIFOCNT7(u16 data) {
    IPCFIFOCNT7 = (IPCFIFOCNT7 & ~0x840C) | (data & 0x840C);

    if (IPCFIFOCNT7 & (1 << 3)) {
        EmptyFIFO7();
    }

    // TODO: check behaviour later
    if ((IPCFIFOCNT7 & (1 << 2)) && (IPCFIFOCNT7 & 1)) {
        core->arm9.SendInterrupt(17);
    }

    if ((IPCFIFOCNT7 & (1 << 10)) && !(IPCFIFOCNT7 & (1 << 8))) {
        core->arm9.SendInterrupt(18);
    }

    // if bit 14 is set then this signifies that the error has been acknowledged
    if (data & (1 << 14)) {
        IPCFIFOCNT7 &= ~(1 << 14);
    }
}

void IPC::WriteIPCFIFOCNT9(u16 data) {
    IPCFIFOCNT9 = (IPCFIFOCNT9 & ~0x840C) | (data & 0x840C);

    if (IPCFIFOCNT9 & (1 << 3)) {
        EmptyFIFO9();
    }

    // TODO: check behaviour later
    if ((IPCFIFOCNT9 & (1 << 2)) && (IPCFIFOCNT9 & 1)) {
        core->arm9.SendInterrupt(17);
    }

    if ((IPCFIFOCNT9 & (1 << 10)) && !(IPCFIFOCNT9 & (1 << 8))) {
        core->arm9.SendInterrupt(18);
    }

    // if bit 14 is set then this signifies that the error has been acknowledged
    if (data & (1 << 14)) {
        IPCFIFOCNT9 &= ~(1 << 14);
    }
}

void IPC::EmptyFIFO7() {
    // create an empty queue and swap it
    std::queue<u32> empty_queue;
    fifo7.swap(empty_queue);
}


void IPC::EmptyFIFO9() {
    // create an empty queue and swap it
    std::queue<u32> empty_queue;
    fifo9.swap(empty_queue);
}
