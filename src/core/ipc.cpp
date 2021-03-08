#include <core/ipc.h>
#include <core/core.h>

IPC::IPC(Core* core) : core(core) {

}

void IPC::Reset() {
    IPCSYNC7 = 0;
    IPCSYNC9 = 0;
    IPCFIFOCNT7 = 0;
    IPCFIFOCNT9 = 0;
    memset(fifo7, 0, 16 * sizeof(u32));
    memset(fifo9, 0, 16 * sizeof(u32));
}

void IPC::WriteIPCSYNC7(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC7 = (IPCSYNC7 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync7 (bits 8..11) to bits 0..3 of ipcsync9
    IPCSYNC9 = (IPCSYNC9 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    printf("counter: %d\n", core->arm7.counter);
    printf("write ipcsync7\n");
    printf("ipcsync7: %08x ipcsync9: %08x\n", IPCSYNC7, IPCSYNC9);
    // if ipcsync 7 is sending an interrupt and interrupts are enabled on ipcsync9
    if ((IPCSYNC7 & (1 << 13)) && (IPCSYNC9 & (1 << 14))) {
        log_fatal("handle ipcsync irqs");
    }
}


void IPC::WriteIPCSYNC9(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC9 = (IPCSYNC9 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync9 (bits 8..11) to bits 0..3 of ipcsync7
    IPCSYNC7 = (IPCSYNC7 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    printf("counter: %d opcode: %08x\n", core->arm9.counter, core->arm9.instruction);
    printf("write ipcsync9\n");
    printf("ipcsync7: %08x ipcsync9: %08x\n", IPCSYNC7, IPCSYNC9);
    // if ipcsync 9 is sending an interrupt and interrupts are enabled on ipcsync 7
    if ((IPCSYNC9 & (1 << 13)) && (IPCSYNC7 & (1 << 14))) {
        log_fatal("handle ipcsync irqs");
    }
}

u16 IPC::ReadIPCSYNC7() {
    return IPCSYNC7 & 0x4F0F;
}

u16 IPC::ReadIPCSYNC9() {
    return IPCSYNC9 & 0x4F0F;
}

void IPC::WriteIPCFIFOCNT7(u16 data) {
    IPCFIFOCNT7 = data & 0xC40C;

    if (IPCFIFOCNT7 & (1 << 3)) {
        memset(fifo7, 0, 16 * sizeof(u32));
    }

    if ((IPCFIFOCNT7 & (1 << 2)) || (IPCFIFOCNT7 & (1 << 10))) {
        log_fatal("handle ipcfifocnt irqs");
    }
}

void IPC::WriteIPCFIFOCNT9(u16 data) {
    IPCFIFOCNT9 = data & 0xC40C;

    if (IPCFIFOCNT9 & (1 << 3)) {
        memset(fifo9, 0, 16 * sizeof(u32));
    }

    if ((IPCFIFOCNT9 & (1 << 2)) || (IPCFIFOCNT9 & (1 << 10))) {
        log_fatal("handle ipcfifocnt irqs");
    }
}