#include <core/hw/dma/dma.h>
#include <core/hw/hw.h>


DMA::DMA(HW* hw, int arch) : hw(hw), arch(arch) {
    for (int i = 0; i < 4; i++) {
        TransferEvent[i] = std::bind(&DMA::Transfer, this, i);
    }
}

void DMA::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&channel[i], 0, sizeof(DMAChannel));
        DMAFILL[i] = 0;
    }
}

void DMA::Transfer(int channel_index) {
    // make variables to make things easier
    u8 destination_control = (channel[channel_index].DMACNT >> 21) & 0x3;
    u8 source_control = (channel[channel_index].DMACNT >> 23) & 0x3;

    u8 start_timing = (channel[channel_index].DMACNT >> 27) & 0x7;
    
    // check the transfer type (either halfwords or words)
    if (channel[channel_index].DMACNT & (1 << 26)) {
        // word transfer
        // loop through all the data units specified by internal length
        for (u32 j = 0; j < channel[channel_index].internal_length; j++) {
            if (arch == 1) {
                hw->arm9_memory.FastWrite<u32>(channel[channel_index].internal_destination, hw->arm9_memory.FastRead<u32>(channel[channel_index].internal_source));
            } else {
                hw->arm7_memory.FastWrite<u32>(channel[channel_index].internal_destination, hw->arm7_memory.FastRead<u32>(channel[channel_index].internal_source));
            }

            // case 2 is just fixed so nothing happens
            switch (source_control) {
            case 0:
                // increment
                channel[channel_index].internal_source += 4;
                break;
            case 1:
                // decrement
                channel[channel_index].internal_source -= 4;
                break;
            case 3:
                log_fatal("prohibited in source control");
                break;
            }

            // case 2 is just fixed so nothing happens
            switch (destination_control) {
            case 0: case 3:
                // increment
                channel[channel_index].internal_destination += 4;
                break;
            case 1:
                // decrement
                channel[channel_index].internal_destination -= 4;
                break;
            case 2:
                // fixed
                break;
            }
        }
    } else {
        // halfword transfer
        for (u32 j = 0; j < channel[channel_index].internal_length; j++) {
            if (arch == 1) {
                hw->arm9_memory.FastWrite<u16>(channel[channel_index].internal_destination, hw->arm9_memory.FastRead<u16>(channel[channel_index].internal_source));
            } else {
                hw->arm7_memory.FastWrite<u16>(channel[channel_index].internal_destination, hw->arm7_memory.FastRead<u16>(channel[channel_index].internal_source));
            }

            // case 2 is just fixed so nothing happens
            switch (source_control) {
            case 0:
                // increment
                channel[channel_index].internal_source += 2;
                break;
            case 1:
                // decrement
                channel[channel_index].internal_source -= 2;
                break;
            case 3:
                log_fatal("prohibited in source control");
                break;
            }

            // case 2 is just fixed so nothing happens
            switch (destination_control) {
            case 0: case 3:
                // increment
                channel[channel_index].internal_destination += 2;
                break;
            case 1:
                // decrement
                channel[channel_index].internal_destination -= 2;
                break;
            case 2:
                // fixed
                break;
            }
        }
    }

    // request dma irq upon end of word count if enabled 
    if (channel[channel_index].DMACNT & (1 << 30)) {
        hw->cpu_core[arch]->SendInterrupt(8 + channel_index);
    }

    if (channel[channel_index].DMACNT & (1 << 25) && start_timing != 0) {
        // restart the internal registers
        channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;

        if (destination_control == 3) {
            // only reload internal destination register in increment/reload mode
            channel[channel_index].internal_destination = channel[channel_index].destination;
        }
    } else {
        // disable the dma channel after the transfer is finished
        channel[channel_index].DMACNT &= ~(1 << 31);
    }
}

void DMA::Trigger(u8 mode) {
    // check each channels start_timing to see if any are equal to mode
    for (int channel_index = 0; channel_index < 4; channel_index++) {
        u8 start_timing;
        if (arch == 1) {
            // for arm9 dma we use bits 27..29
            start_timing = (channel[channel_index].DMACNT >> 27) & 0x7;
        } else {
            // for arm7 dma we use bits 28..29
            start_timing = (channel[channel_index].DMACNT >> 28) & 0x3;
        }
        if ((channel[channel_index].DMACNT & (1 << 31)) && (start_timing == mode)) {
            // activate that dma channel
            hw->scheduler.Add(1, TransferEvent[channel_index]);
        }
    }
}

void DMA::WriteDMACNT_L(int channel_index, u16 data) {
    // write to the lower 16 bits of DMACNT
    channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFFF) | (data & 0xFFFF);
}

void DMA::WriteDMACNT_H(int channel_index, u16 data) {
    // write to the upper 16 bits of DMACNT
    // so this will include bits 16..20 of the Length and then all the bits of Control
    u32 old_DMACNT = channel[channel_index].DMACNT;

    channel[channel_index].DMACNT = (channel[channel_index].DMACNT & 0xFFFF) | (data << 16);

    u8 start_timing = (channel[channel_index].DMACNT >> 27) & 0x7;

    // don't load internal registers if enable bit wasn't changed from 0 to 1
    if ((old_DMACNT & (1 << 31)) || !(channel[channel_index].DMACNT & (1 << 31))) {
        return;
    }

    // otherwise load internal registers
    channel[channel_index].internal_source = channel[channel_index].source;
    channel[channel_index].internal_destination = channel[channel_index].destination;

    if (arch == 1) {
        if ((channel[channel_index].DMACNT & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x200000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;
        }
    } else {
        if ((channel[channel_index].DMACNT & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x10000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;
        }
    }

    if (start_timing == 0) {
        hw->scheduler.Add(1, TransferEvent[channel_index]);
    }
}

void DMA::WriteDMACNT(int channel_index, u32 data) {
    // do a write length but only give bits 0..20 as that is what the word count occupies in dmacnt
    WriteDMACNT_L(channel_index, data & 0xFFFF);
    WriteDMACNT_H(channel_index, data >> 16);
}

auto DMA::ReadDMACNT(int channel_index) -> u32 {
    return channel[channel_index].DMACNT;
}

auto DMA::ReadDMACNT_L(int channel_index) -> u16 {
    return channel[channel_index].DMACNT & 0xFFFF;
}

auto DMA::ReadDMACNT_H(int channel_index) -> u16 {
    return channel[channel_index].DMACNT >> 16;
}

void DMA::WriteLength(int channel_index, u32 data) {
    if (arch == 1) {
        // arm9 dma
        if (data == 0) {
            // 0 = 0x200000
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & 0xFFC00000) | (0x200000);
        } else {
            // 0x1..0x1FFFFF
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0x1FFFFF) | (data & 0x1FFFFF);
        }
    } else {
        // arm7 dma
        if (data == 0) {
            // 0 = 0x10000 on channel 3 and 0x4000 on all other channels
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & 0xFFE00000) | ((channel_index == 3) ? 0x10000 : 0x4000);
        } else {
            // 0x1..0xFFFF on channel 3 and 0x1..0x3FFF on all other channels
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFFF) | (data & ((channel_index == 3) ? 0xFFFF : 0x3FFF));
        }
    }
}

auto DMA::ReadLength(int channel_index) -> u32 {
    // get bits 0..20 of dmacnt
    return channel[channel_index].DMACNT & 0x1FFFFF;
}