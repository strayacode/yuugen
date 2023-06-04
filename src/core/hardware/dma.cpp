#include "common/logger.h"
#include "core/hardware/dma.h"

namespace core {

DMA::DMA(arm::Memory& memory, IRQ& irq, arm::Arch arch) : memory(memory), irq(irq), arch(arch) {}

void DMA::reset() {
    channels.fill(Channel{});
    dmafill.fill(0);
}

void DMA::write_length(int index, u16 value, u32 mask) {
    channels[index].length = (channels[index].length & ~mask) | (value & mask);
}

void DMA::write_source(int index, u32 value, u32 mask) {
    channels[index].source = (channels[index].source & ~mask) | (value & mask);
}

void DMA::write_destination(int index, u32 value, u32 mask) {
    channels[index].destination = (channels[index].destination & ~mask) | (value & mask);
}

void DMA::write_control(int index, u16 value, u32 mask) {
    auto& channel = channels[index];
    auto old_control = channel.control;

    channel.length &= 0xffff;
    channel.length |= value & 0x1f & mask;
    channel.control.data = (channel.control.data & ~mask) | (value & mask);

    if (channel.control.enable && channel.control.timing == Timing::GXFIFO) {
        logger.todo("DMA: handle gxfifo dmas");
    }

    if (old_control.enable || !channel.control.enable) {
        return;
    }

    channel.internal_source = channel.source;
    channel.internal_destination = channel.destination;

    if (channel.length == 0) {
        if (arch == arm::Arch::ARMv5) {
            channel.internal_length = 0x200000;
        } else {
            channel.internal_length = 0x10000;
        }
    } else {
        channel.internal_length = channel.length;
    }

    if (channel.control.timing == Timing::Immediate) {
        transfer(index);
    }
}

void DMA::transfer(int index) {
    // TODO: handle gxfifo transfer limit
    auto& channel = channels[index];
    auto source_adjust = adjust_lut[channel.control.transfer_words][channel.control.source_control];
    auto destination_adjust = adjust_lut[channel.control.transfer_words][channel.control.destination_control];
    
    if (channel.control.transfer_words) {
        for (u32 i = 0; i < channel.internal_length; i++) {
            memory.write<u32, arm::Bus::System>(channel.internal_destination, memory.read<u32, arm::Bus::System>(channel.internal_source));
            channel.internal_source += source_adjust;
            channel.internal_destination += destination_adjust;
        }
    } else {
        for (u32 i = 0; i < channel.internal_length; i++) {
            memory.write<u16, arm::Bus::System>(channel.internal_destination, memory.read<u16, arm::Bus::System>(channel.internal_source));
            channel.internal_source += source_adjust;
            channel.internal_destination += destination_adjust;
        }
    }

    if (channel.control.irq) {
        switch (index) {
        case 0:
            irq.raise(IRQ::Source::DMA0);
            break;
        case 1:
            irq.raise(IRQ::Source::DMA1);
            break;
        case 2:
            irq.raise(IRQ::Source::DMA2);
            break;
        case 3:
            irq.raise(IRQ::Source::DMA3);
            break;
        }
    }

    if (channel.control.repeat && channel.control.timing != Timing::Immediate) {
        channel.internal_length = channel.length;
        
        if (channel.control.destination_control == AddressMode::Reload) {
            channel.internal_destination = channel.destination;
        }
        
        if (channel.control.timing == Timing::GXFIFO) {
            logger.todo("DMA: handle gxfifo transfers");
        }
    } else {
        channel.control.enable = false;
    }
}

} // namespace core