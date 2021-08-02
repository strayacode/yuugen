#include <core/hw/ipc/ipc.h>
#include <core/hw/hw.h>

IPC::IPC(HW* hw) : hw(hw) {

}

void IPC::Reset() {
    IPCSYNC7 = 0;
    IPCSYNC9 = 0;
    IPCFIFOCNT7 = 0x101;
    IPCFIFOCNT9 = 0x101;
    fifo7recv = 0;
    fifo9recv = 0;
}

void IPC::WriteIPCSYNC7(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC7 = (IPCSYNC7 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync7 (bits 8..11) to bits 0..3 of ipcsync9
    IPCSYNC9 = (IPCSYNC9 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    
    // if ipcsync 7 is sending an interrupt and interrupts are enabled on ipcsync9
    if ((IPCSYNC7 & (1 << 13)) && (IPCSYNC9 & (1 << 14))) {
        hw->arm9.SendInterrupt(16);
    }
}


void IPC::WriteIPCSYNC9(u16 data) {
    // write to bits 8..11, 13 and 14
    // also we must make sure to preserve bits 0..3
    IPCSYNC9 = (IPCSYNC9 & ~0x6F00) | (data & 0x6F00);

    // then send the output of ipcsync9 (bits 8..11) to bits 0..3 of ipcsync7
    IPCSYNC7 = (IPCSYNC7 & ~((0x6F00 >> 8) & 0xF)) | (((data & 0x6F00) >> 8) & 0xF);
    
    // if ipcsync 9 is sending an interrupt and interrupts are enabled on ipcsync 7
    if ((IPCSYNC9 & (1 << 13)) && (IPCSYNC7 & (1 << 14))) {
        hw->arm7.SendInterrupt(16);
    }
}

auto IPC::ReadIPCSYNC7() -> u16 {
    return IPCSYNC7 & 0x4F0F;
}

auto IPC::ReadIPCSYNC9() -> u16 {
    return IPCSYNC9 & 0x4F0F;
}

void IPC::WriteIPCFIFOCNT7(u16 data) {
    IPCFIFOCNT7 = (IPCFIFOCNT7 & ~0x8404) | (data & 0x8404);

    // don't set bit 3 in ipcfifocnt
    // it's purpose is to empty fifo
    if (data & (1 << 3)) {
        EmptyFIFO7();
        // set send fifo empty status to set
        IPCFIFOCNT7 |= 1;
        // reset send fifo full status to set
        IPCFIFOCNT7 &= ~(1 << 1);

        IPCFIFOCNT9 |= (1 << 8);
        IPCFIFOCNT9 &= ~(1 << 9);

        // request a Send Fifo Empty IRQ if bit 2 is set
        if (IPCFIFOCNT7 & (1 << 2)) {
            hw->arm7.SendInterrupt(17);
        }

    }

    // request a send fifo empty irq if (bit 2 and bit 0) goes from 0 to 1
    if (!(IPCFIFOCNT7 & (1 << 2)) && (IPCFIFOCNT7 & 1) && (data & (1 << 2))) {
        hw->arm7.SendInterrupt(17);
    }

    // request a fifo recv not empty irq if (bit 10 and not bit 8) goes from 0 to 1
    if (!(IPCFIFOCNT7 & (1 << 10)) && !(IPCFIFOCNT7 & (1 << 8)) && (data & (1 << 10))) {
        hw->arm7.SendInterrupt(18);
    }

    // if bit 14 is set then this signifies that the error has been acknowledged
    if (data & (1 << 14)) {
        IPCFIFOCNT7 &= ~(1 << 14);
    }
}

void IPC::WriteIPCFIFOCNT9(u16 data) {
    IPCFIFOCNT9 = (IPCFIFOCNT9 & ~0x8404) | (data & 0x8404);

    // don't set bit 3 in ipcfifocnt
    // it's purpose is to empty fifo
    if (data & (1 << 3)) {
        EmptyFIFO9();
        // set send fifo empty status to set
        IPCFIFOCNT9 |= 1;
        // reset send fifo full status to set
        IPCFIFOCNT9 &= ~(1 << 1);

        IPCFIFOCNT7 |= (1 << 8);
        IPCFIFOCNT7 &= ~(1 << 9);

        // request a Send Fifo Empty IRQ if bit 2 is set
        if (IPCFIFOCNT9 & (1 << 2)) {
            hw->arm9.SendInterrupt(17);
        }
    }

    // request a send fifo empty irq if (bit 2 and bit 0) goes from 0 to 1
    if (!(IPCFIFOCNT9 & (1 << 2)) && (IPCFIFOCNT9 & 1) && (data & (1 << 2))) {
        hw->arm9.SendInterrupt(17);
    }

    // request a fifo recv not empty irq if (bit 10 and not bit 8) goes from 0 to 1
    if (!(IPCFIFOCNT9 & (1 << 10)) && !(IPCFIFOCNT9 & (1 << 8)) && (data & (1 << 10))) {
        hw->arm9.SendInterrupt(18);
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

auto IPC::ReadFIFORECV7() -> u32 {
    if (!fifo9.empty()) {
        // get the first word
        fifo7recv = fifo9.front();
        if (IPCFIFOCNT7 & (1 << 15)) {
            // remove the word we recieved from the send fifo
            // of the other cpu
            fifo9.pop();

            // check if send fifo of other cpu is empty
            if (fifo9.empty()) {
                // set respective bits
                IPCFIFOCNT9 |= 1;
                IPCFIFOCNT7 |= (1 << 8);
                // trigger a recieve fifo empty irq if enabled
                // on the other ipcfifocnt
                if (IPCFIFOCNT9 & (1 << 2)) {
                    hw->arm9.SendInterrupt(17);
                }
            } else if (fifo9.size() == 15) {
                // recieve fifo now went from full to not full
                IPCFIFOCNT9 &= ~(1 << 1);
                IPCFIFOCNT7 &= ~(1 << 9);
            }
        }
    } else {
        // set the error bit as reading had an empty fifo
        IPCFIFOCNT7 |= (1 << 14);
    }
    return fifo7recv;
    
}

auto IPC::ReadFIFORECV9() -> u32 {
    if (!fifo7.empty()) {
        // get the first word
        fifo9recv = fifo7.front();
        if (IPCFIFOCNT9 & (1 << 15)) {
            // remove the word we recieved from the send fifo
            // of the other cpu
            fifo7.pop();

            // check if send fifo of other cpu is empty
            if (fifo7.empty()) {
                // set respective bits
                IPCFIFOCNT7 |= 1;
                IPCFIFOCNT9 |= (1 << 8);
                // trigger a recieve fifo empty irq if enabled
                // on the other ipcfifocnt
                if (IPCFIFOCNT7 & (1 << 2)) {
                    hw->arm7.SendInterrupt(17);
                }
            } else if (fifo7.size() == 15) {
                // recieve fifo now went from full to not full
                IPCFIFOCNT7 &= ~(1 << 1);
                IPCFIFOCNT9 &= ~(1 << 9);
            }
        }
    } else {
        // set the error bit as reading had an empty fifo
        IPCFIFOCNT9 |= (1 << 14);
    }
    return fifo9recv;
}

void IPC::WriteFIFOSEND7(u32 data) {
    // only write if send and recieve fifo is enabled
    if (IPCFIFOCNT7 & (1 << 15)) {
        if (fifo7.size() < 16) {
            // push a word to the fifo
            fifo7.push(data);

            if (fifo7.size() == 1) {
                // now the send fifo is not empty anymore
                IPCFIFOCNT7 &= ~1;
                IPCFIFOCNT9 &= ~(1 << 8);
                if (IPCFIFOCNT9 & (1 << 10)) {
                    // send recv fifo not empty irq to other cpu
                    hw->arm9.SendInterrupt(18);
                }
            } else if (fifo7.size() == 16) {
                // set the full bits since fifo is full
                // set send fifo full status
                IPCFIFOCNT7 |= (1 << 1);
                IPCFIFOCNT9 |= (1 << 9);
            }
        } else {
            // set the error fifo send full bit in ipcfifocnt
            IPCFIFOCNT7 |= (1 << 14);
        }
    }
}

// write a word to the fifo
void IPC::WriteFIFOSEND9(u32 data) {
    // only write if send and recieve fifo is enabled
    if (IPCFIFOCNT9 & (1 << 15)) {
        if (fifo9.size() < 16) {
            // push a word to the fifo
            fifo9.push(data);
            if (fifo9.size() == 1) {
                IPCFIFOCNT9 &= ~1;
                IPCFIFOCNT7 &= ~(1 << 8);

                if (IPCFIFOCNT7 & (1 << 10)) {
                    // send recv fifo not empty irq to other cpu
                    hw->arm7.SendInterrupt(18);
                }
            } else if (fifo9.size() == 16) {
                // set the full bits since fifo is full
                // set send fifo full status
                IPCFIFOCNT9 |= (1 << 1);
                IPCFIFOCNT7 |= (1 << 9);
            }
        } else {
            // set the error fifo send full bit in ipcfifocnt
            IPCFIFOCNT9 |= (1 << 14);
        }
    }
}
