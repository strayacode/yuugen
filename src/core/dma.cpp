#include <core/core.h>
#include <core/dma.h>


DMA::DMA(Core* core, int arch) : core(core), arch(arch) {

}

void DMA::Reset() {
    for (int i = 0; i < 4; i++) {
        memset(&channel[i], 0, sizeof(DMAChannel));
        DMAFILL[i] = 0;
    }

    enabled = 0;
}

void DMA::Transfer() {
    // check each dma channel if they are enabled, starting with channel 0 first as that has the highest priority
    for (int i = 0; i < 4; i++) {
        // only do transfer if enabled obviously
        if (enabled & (1 << i)) {
            // make variables to make things easier
            u8 destination_control = (channel[i].DMACNT >> 21) & 0x7;
            u8 source_control = (channel[i].DMACNT >> 23) & 0x7;
            if (channel[i].DMACNT & (1 << 25)) {
                log_fatal("implement support for dma repeat");
            }

            // loop through all the data units specified by internal length
            for (u32 j = 0; j < channel[i].internal_length; j++) {
                // check the transfer type (either halfwords or words)
                if (channel[i].DMACNT & (1 << 26)) {
                    // word transfer
                    if (arch == ARMv5) {
                        core->memory.ARM9WriteWord(channel[i].internal_destination, core->memory.ARM9ReadWord(channel[i].internal_source));
                    } else {
                        core->memory.ARM7WriteWord(channel[i].internal_destination, core->memory.ARM7ReadWord(channel[i].internal_source));
                    }

                    // case 2 is just fixed so nothing happens
                    switch (destination_control) {
                    case 0:
                        // increment
                        channel[i].internal_destination += 4;
                        break;
                    case 1:
                        // decrement
                        channel[i].internal_destination -= 4;
                        break;
                    case 3:
                        log_fatal("handle increment/reload in destination control");
                        break;
                    }

                    // case 2 is just fixed so nothing happens
                    switch (source_control) {
                    case 0:
                        // increment
                        channel[i].internal_source += 4;
                        break;
                    case 1:
                        // decrement
                        channel[i].internal_source -= 4;
                        break;
                    case 3:
                        log_fatal("prohibited in destination control");
                        break;
                    }

                } else {
                    // halfword transfer
                    if (arch == ARMv5) {
                        core->memory.ARM9WriteHalfword(channel[i].internal_destination, core->memory.ARM9ReadHalfword(channel[i].internal_source));
                    } else {
                        core->memory.ARM7WriteHalfword(channel[i].internal_destination, core->memory.ARM7ReadHalfword(channel[i].internal_source));
                    }

                    // case 2 is just fixed so nothing happens
                    switch (destination_control) {
                    case 0:
                        // increment
                        channel[i].internal_destination += 2;
                        break;
                    case 1:
                        // decrement
                        channel[i].internal_destination -= 2;
                        break;
                    case 3:
                        log_fatal("handle increment/reload in destination control");
                        break;
                    }

                    // case 2 is just fixed so nothing happens
                    switch (source_control) {
                    case 0:
                        // increment
                        channel[i].internal_source += 2;
                        break;
                    case 1:
                        // decrement
                        channel[i].internal_source -= 2;
                        break;
                    case 3:
                        log_fatal("prohibited in destination control");
                        break;
                    }
                }


            }
            // request dma irq upon end of word count if enabled 
            if (channel[i].DMACNT & (1 << 30)) {
                if (arch == 1) {
                    // arm9
                    core->arm9.SendInterrupt(8 + i);
                } else {
                    // arm7
                    core->arm7.SendInterrupt(8 + i);
                }
            }

            // disable the dma channel after the transfer is finished
            enabled &= ~(1 << i);
            channel[i].DMACNT &= ~(1 << 31);

        }
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
            enabled |= (1 << channel_index);
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

    // enable bit gets turned off alter the appropriate bit in enabled
    if (!(channel[channel_index].DMACNT & (1 << 31))) {
        enabled &= ~(1 << channel_index);
    }

    // don't load internal registers if enable bit wasn't changed from 0 to 1
    if ((old_DMACNT & (1 << 31)) || !(channel[channel_index].DMACNT & (1 << 31))) {
        return;
    }

    // otherwise load internal registers

    channel[channel_index].internal_source = channel[channel_index].source;
    channel[channel_index].internal_destination = channel[channel_index].destination;

    if (arch == 1) {
        // arm9
        if ((channel[channel_index].DMACNT & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x200000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;
        }
    } else {
        // arm7
        if ((channel[channel_index].DMACNT & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x10000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;
        }
    }

    if (start_timing == 0) {
        enabled |= (1 << channel_index);
    }
}

void DMA::WriteDMACNT(int channel_index, u32 data) {
    // do a write length but only give bits 0..20 as that is what the word count occupies in dmacnt
    WriteDMACNT_L(channel_index, data & 0xFFFF);
    WriteDMACNT_H(channel_index, data >> 16);
}

u32 DMA::ReadDMACNT(int channel_index) {
    return channel[channel_index].DMACNT;
}

u16 DMA::ReadDMACNT_L(int channel_index) {
    return channel[channel_index].DMACNT & 0xFFFF;
}

u16 DMA::ReadDMACNT_H(int channel_index) {
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

u32 DMA::ReadLength(int channel_index) {
    // get bits 0..20 of dmacnt
    return channel[channel_index].DMACNT & 0x1FFFFF;
}