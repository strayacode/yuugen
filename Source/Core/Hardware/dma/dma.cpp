#include <string>
#include "Core/Hardware/dma/dma.h"
#include "Core/Core.h"

DMA::DMA(System& system, int arch) : system(system), arch(arch) {}

void DMA::reset() {
    for (int i = 0; i < 4; i++) {
        memset(&channel[i], 0, sizeof(DMAChannel));
        DMAFILL[i] = 0;
    }

    for (int i = 0; i < 4; i++) {
        transfer_event[i] = system.scheduler.RegisterEvent("DMATransfer" + std::to_string(i), [this, i]() {
            Transfer(i);
        });
    }
}

void DMA::build_mmio(MMIO& mmio, Arch arch) {
    int channel_size = 12;

    for (int i = 0; i < 4; i++) {
        mmio.register_mmio<u32>(
            0x040000b0 + (channel_size * i),
            mmio.direct_read<u32>(&channel[i].source),
            mmio.direct_write<u32>(&channel[i].source)
        );

        mmio.register_mmio<u32>(
            0x040000b4 + (channel_size * i),
            mmio.direct_read<u32>(&channel[i].destination),
            mmio.direct_write<u32>(&channel[i].destination)
        );

        mmio.register_mmio<u16>(
            0x040000b8 + (channel_size * i),
            mmio.invalid_read<u16>(),
            mmio.complex_write<u16>([this, i](u32, u16 data) {
                write_dma_length(i, data);
            })
        );

        mmio.register_mmio<u32>(
            0x040000b8 + (channel_size * i),
            mmio.direct_read<u32>(&channel[i].dmacnt),
            mmio.complex_write<u32>([this, i](u32, u32 data) {
                write_dma_length(i, data & 0xFFFF);
                write_dma_control(i, data >> 16);
            })
        );

        mmio.register_mmio<u16>(
            0x040000ba + (channel_size * i),
            mmio.complex_read<u16>([this, i](u32) {
                return channel[i].dmacnt >> 16;
            }),
            mmio.complex_write<u16>([this, i](u32, u16 data) {
                write_dma_control(i, data);
            })
        );
    }
}

void DMA::Transfer(int channel_index) {
    u8 destination_control = (channel[channel_index].dmacnt >> 21) & 0x3;
    u8 source_control = (channel[channel_index].dmacnt >> 23) & 0x3;
    u8 mode = (channel[channel_index].dmacnt >> 27) & 0x7;
    int source_adjust = adjust_lut[(channel[channel_index].dmacnt >> 26) & 0x1][source_control];
    int destination_adjust = adjust_lut[(channel[channel_index].dmacnt >> 26) & 0x1][destination_control];
    int gxfifo_transfers = 0;
    
    if (channel[channel_index].dmacnt & (1 << 26)) {
        // word transfer
        // loop through all the data units specified by internal length
        for (u32 j = 0; j < channel[channel_index].internal_length; j++) {
            if (arch == 1) {
                system.arm9_memory.write<u32>(channel[channel_index].internal_destination, system.arm9_memory.read<u32>(channel[channel_index].internal_source));
            } else {
                system.arm7_memory.write<u32>(channel[channel_index].internal_destination, system.arm7_memory.read<u32>(channel[channel_index].internal_source));
            }

            channel[channel_index].internal_source += source_adjust;
            channel[channel_index].internal_destination += destination_adjust;

            // gxfifo dma can only transfer 112 words
            if (mode == 7 && ++gxfifo_transfers == 112) break;
        }
    } else {
        // halfword transfer
        for (u32 j = 0; j < channel[channel_index].internal_length; j++) {
            if (arch == 1) {
                system.arm9_memory.write<u16>(channel[channel_index].internal_destination, system.arm9_memory.read<u16>(channel[channel_index].internal_source));
            } else {
                system.arm7_memory.write<u16>(channel[channel_index].internal_destination, system.arm7_memory.read<u16>(channel[channel_index].internal_source));
            }

            channel[channel_index].internal_source += source_adjust;
            channel[channel_index].internal_destination += destination_adjust;

            // gxfifo dma can only transfer 112 words
            if (mode == 7 && ++gxfifo_transfers == 112) break;
        }
    }

    // request dma irq upon end of word count if enabled 
    if (channel[channel_index].dmacnt & (1 << 30)) {
        switch (channel_index) {
        case 0:
            system.cpu_core[arch].SendInterrupt(InterruptType::DMA0);
            break;
        case 1:
            system.cpu_core[arch].SendInterrupt(InterruptType::DMA1);
            break;
        case 2:
            system.cpu_core[arch].SendInterrupt(InterruptType::DMA2);
            break;
        case 3:
            system.cpu_core[arch].SendInterrupt(InterruptType::DMA3);
            break;
        }
    }

    if (channel[channel_index].dmacnt & (1 << 25) && mode != 0) {
        // restart the internal registers
        channel[channel_index].internal_length = channel[channel_index].dmacnt & 0x1FFFFF;

        if (destination_control == 3) {
            // only reload internal destination register in increment/reload mode
            channel[channel_index].internal_destination = channel[channel_index].destination;
        }

        // schedule another gxfifo dma if the gxfifo is still half empty
        if (mode == 7 && system.video_unit.renderer_3d.gxfifo_half_empty()) {
            system.scheduler.AddEvent(1, &transfer_event[channel_index]);
        }
    } else {
        // disable the dma channel after the transfer is finished
        channel[channel_index].dmacnt &= ~(1 << 31);
    }
}

void DMA::Trigger(u8 mode) {
    // check each channels start_timing to see if any are equal to mode
    for (int channel_index = 0; channel_index < 4; channel_index++) {
        u8 start_timing;
        if (arch == 1) {
            // for arm9 dma we use bits 27..29
            start_timing = (channel[channel_index].dmacnt >> 27) & 0x7;
        } else {
            // for arm7 dma we use bits 28..29
            start_timing = (channel[channel_index].dmacnt >> 28) & 0x3;
        }
        if ((channel[channel_index].dmacnt & (1 << 31)) && (start_timing == mode)) {
            system.scheduler.AddEvent(1, &transfer_event[channel_index]);
        }
    }
}

void DMA::write_dma_length(int channel_index, u16 data) {
    // write to the lower 16 bits of dmacnt
    channel[channel_index].dmacnt = (channel[channel_index].dmacnt & ~0xFFFF) | (data & 0xFFFF);
}

void DMA::write_dma_control(int channel_index, u16 data) {
    // write to the upper 16 bits of dmacnt
    // so this will include bits 16..20 of the Length and then all the bits of Control
    u32 old_DMACNT = channel[channel_index].dmacnt;

    channel[channel_index].dmacnt = (channel[channel_index].dmacnt & 0xFFFF) | (data << 16);

    u8 start_timing = (channel[channel_index].dmacnt >> 27) & 0x7;

    // immediately schedule a dma transfer if the gxfifo is half empty
    if ((channel[channel_index].dmacnt & (1 << 31)) && (start_timing == 7) && system.video_unit.renderer_3d.gxfifo_half_empty()) {
        system.scheduler.AddEvent(1, &transfer_event[channel_index]);
    }

    // don't load internal registers if enable bit wasn't changed from 0 to 1
    if ((old_DMACNT & (1 << 31)) || !(channel[channel_index].dmacnt & (1 << 31))) {
        return;
    }

    // otherwise load internal registers
    channel[channel_index].internal_source = channel[channel_index].source;
    channel[channel_index].internal_destination = channel[channel_index].destination;

    if (arch == 1) {
        if ((channel[channel_index].dmacnt & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x200000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].dmacnt & 0x1FFFFF;
        }
    } else {
        if ((channel[channel_index].dmacnt & 0x1FFFFF) == 0) {
            channel[channel_index].internal_length = 0x10000;
        } else {
            channel[channel_index].internal_length = channel[channel_index].dmacnt & 0x1FFFFF;
        }
    }

    if (start_timing == 0) {
        system.scheduler.AddEvent(1, &transfer_event[channel_index]);
    }
}

void DMA::WriteLength(int channel_index, u32 data) {
    if (arch == 1) {
        // arm9 dma
        if (data == 0) {
            // 0 = 0x200000
            channel[channel_index].dmacnt = (channel[channel_index].dmacnt & 0xFFC00000) | 0x200000;
        } else {
            // 0x1..0x1FFFFF
            channel[channel_index].dmacnt = (channel[channel_index].dmacnt & ~0x1FFFFF) | (data & 0x1FFFFF);
        }
    } else {
        // arm7 dma
        if (data == 0) {
            // 0 = 0x10000 on channel 3 and 0x4000 on all other channels
            channel[channel_index].dmacnt = (channel[channel_index].dmacnt & 0xFFE00000) | ((channel_index == 3) ? 0x10000 : 0x4000);
        } else {
            // 0x1..0xFFFF on channel 3 and 0x1..0x3FFF on all other channels
            channel[channel_index].dmacnt = (channel[channel_index].dmacnt & ~0xFFFF) | (data & ((channel_index == 3) ? 0xFFFF : 0x3FFF));
        }
    }
}

u32 DMA::ReadLength(int channel_index) {
    // get bits 0..20 of dmacnt
    return channel[channel_index].dmacnt & 0x1FFFFF;
}