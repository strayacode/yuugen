#pragma once

#include <array>
#include "common/types.h"
#include "common/ring_buffer.h"
#include "arm/arch.h"
#include "core/hardware/irq.h"

namespace core {

class IPC {
public:
    IPC(IRQ& irq7, IRQ& irq9);

    void reset();

    u32 read_ipcsync(arm::Arch arch);
    u16 read_ipcfifocnt(arm::Arch arch);
    u32 read_ipcfiforecv(arm::Arch arch);

    void write_ipcsync(arm::Arch arch, u32 value);
    void write_ipcfifocnt(arm::Arch arch, u16 value);
    void write_ipcfifosend(arm::Arch arch, u32 value);

private:
    union IPCSYNC {
        struct {
            u8 input : 4;
            u32 : 4;
            u8 output : 4;
            bool : 1;
            bool send_irq : 1;
            bool enable_irq : 1;
            u32 : 17;
        };

        u32 data;
    };

    union IPCFIFOCNT {
        struct {
            bool send_fifo_empty : 1;
            bool send_fifo_full : 1;
            bool send_fifo_empty_irq : 1;
            bool send_fifo_clear : 1;
            u32 : 4;
            bool receive_fifo_empty : 1;
            bool receive_fifo_full : 1;
            bool receive_fifo_empty_irq : 1;
            u32 : 3;
            bool error : 1;
            bool enable_fifos : 1;
        };

        u16 data;
    };

    std::array<IPCSYNC, 2> ipcsync;
    std::array<IPCFIFOCNT, 2> ipcfifocnt;
    std::array<common::RingBuffer<u32, 16>, 2> fifo;
    std::array<u32, 2> ipcfiforecv;
    std::array<IRQ*, 2> irq;
};

} // namespace core