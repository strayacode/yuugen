#include "common/logger.h"
#include "gba/hardware/dma.h"

namespace gba {

DMA::DMA(common::Scheduler& scheduler, arm::Memory& memory, IRQ& irq) : scheduler(scheduler), memory(memory), irq(irq) {}

void DMA::reset() {
    channels.fill(Channel{});
    
    for (int i = 0; i < 4; i++) {
        transfer_events[i] = scheduler.register_event("DMA Transfer", [this, i]() {
            transfer(i);
        });
    }
}

void DMA::trigger(Timing timing) {
    for (int i = 0; i < 4; i++) {
        auto& channel = channels[i];
        if (channel.control.enable && channel.control.timing == timing) {
            scheduler.add_event(1, &transfer_events[i]);
        }
    }
}

u16 DMA::read_length(int id) {
    return channels[id].length;
}

u16 DMA::read_control(int id) {
    return channels[id].control.data;
}

void DMA::write_length(int id, u16 value, u32 mask) {
    auto& channel = channels[id];
    channel.length = (channel.length & ~mask) | (value & mask);

    if (channel.length == 0) {
        if (id == 3) {
            channel.internal_length = 0x10000;
        } else {
            channel.internal_length = 0x4000;
        }
    } else {
        channel.internal_length = channel.length;
    }
}

void DMA::write_source(int id, u32 value, u32 mask) {
    channels[id].source = (channels[id].source & ~mask) | (value & mask);
}

void DMA::write_destination(int id, u32 value, u32 mask) {
    channels[id].destination = (channels[id].destination & ~mask) | (value & mask);
}

void DMA::write_control(int id, u16 value, u32 mask) {
    auto& channel = channels[id];
    auto old_control = channel.control;

    channel.control.data = (channel.control.data & ~mask) | (value & mask);

    if (old_control.enable || !channel.control.enable) {
        return;
    }

    channel.internal_source = channel.source;
    channel.internal_destination = channel.destination;

    if (channel.control.timing == Timing::Immediate) {
        scheduler.add_event(1, &transfer_events[id]);
    }
}

void DMA::transfer(int id) {
    auto& channel = channels[id];
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
        switch (id) {
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
    } else {
        channel.control.enable = false;
    }
}

} // namespace gba