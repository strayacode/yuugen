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
            if ((channel[i].DMACNT & (1 << 25)) || (channel[i].DMACNT & (1 << 30))) {
                log_fatal("repeat bit or irq request bit not implemented!");
            }

            // loop through all the data units specified by internal length
            for (int j = 0; j < channel[i].internal_length; j++) {
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
            // disable the dma channel after the transfer is finished
            enabled &= ~(1 << i);
            channel[i].DMACNT &= ~(1 << 31);

        }
    }
}

void DMA::WriteLength(int channel_index, u32 data) {
    if (arch == 1) {
        // arm9 dma
        if (data == 0) {
            // 0 = 0x200000
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFC00000) | (0x200000);
        } else {
            // 0x1..0x1FFFFF
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0x1FFFFF) | (data & 0x1FFFFF);
        }
    } else {
        // arm7 dma
        if (data == 0) {
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFE00000) | ((channel_index == 3) ? 0x10000 : 0x4000);
        } else {
            channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFFF) | (data & ((channel_index == 3) ? 0xFFFF : 0x3FFF));
        }
    }
}

void DMA::WriteControl(int channel_index, u32 data) {
    u32 old_DMACNT = channel[channel_index].DMACNT;

    channel[channel_index].DMACNT = (channel[channel_index].DMACNT & ~0xFFE00000) | (data & 0xFFE00000);

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
    channel[channel_index].internal_length = channel[channel_index].DMACNT & 0x1FFFFF;

    // enable channel immediately if start timing is 0 so a dma transfer will occur on the next cpu step
    if (start_timing == 0) {
        enabled |= (1 << channel_index);
    } else {
        log_fatal("start timing %d has not been implemented yet for channel %d", start_timing, channel_index);
    }
}

void DMA::WriteDMACNT(int channel_index, u32 data) {
    // do a write length but only give bits 0..20 as that is what the word count occupies in dmacnt
    WriteLength(channel_index, data & 0x1FFFFF);
    WriteControl(channel_index, data);
}