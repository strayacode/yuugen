#include "common/bits.h"
#include "core/hardware/ipc.h"

namespace core {

IPC::IPC(IRQ& irq7, IRQ& irq9) {
    irq[0] = &irq7;
    irq[1] = &irq9;
}

void IPC::reset() {
    ipcsync.fill(IPCSYNC{});
    ipcfifocnt[0].data = 0x101;
    ipcfifocnt[1].data = 0x101;
    fifo[0].reset();
    fifo[1].reset();
    ipcfiforecv.fill(0);
}

u32 IPC::read_ipcsync(arm::Arch arch) {
    return ipcsync[static_cast<int>(arch)].data;
}

u16 IPC::read_ipcfifocnt(arm::Arch arch) {
    return ipcfifocnt[static_cast<int>(arch)].data;
}

void IPC::write_ipcsync(arm::Arch arch, u32 value) {
    auto& tx = ipcsync[static_cast<int>(arch)];
    auto& rx = ipcsync[!static_cast<int>(arch)];
    auto rx_irq = irq[!static_cast<int>(arch)];
    
    tx.data = (tx.data & ~0x6f00) | (value & 0x6f00);
    rx.input = tx.output;

    if (tx.send_irq && rx.enable_irq) {
        rx_irq->raise(IRQ::Source::IPCSync);
    }
}

void IPC::write_ipcfifocnt(arm::Arch arch, u16 value) {
    auto& tx = ipcfifocnt[static_cast<int>(arch)];
    auto& tx_fifo = fifo[static_cast<int>(arch)];
    auto tx_irq = irq[static_cast<int>(arch)];
    auto& rx = ipcfifocnt[!static_cast<int>(arch)];
    bool send_fifo_empty_irq_old = tx.send_fifo_empty_irq;
    bool receive_fifo_empty_irq_old = tx.receive_fifo_empty_irq;

    tx.data = (tx.data & ~0x8404) | (value & 0x8404);

    if (common::get_bit<3>(value)) {
        tx_fifo.reset();
        tx.send_fifo_empty = true;
        tx.send_fifo_full = false;
        rx.receive_fifo_empty = true;
        rx.receive_fifo_full = false;

        if (tx.send_fifo_empty_irq) {
            tx_irq->raise(IRQ::Source::IPCSendEmpty);
        }
    }

    if (!send_fifo_empty_irq_old && tx.send_fifo_empty_irq && tx.send_fifo_empty) {
        tx_irq->raise(IRQ::Source::IPCSendEmpty);
    }

    if (!receive_fifo_empty_irq_old && tx.receive_fifo_empty_irq && tx.receive_fifo_empty) {
        tx_irq->raise(IRQ::Source::IPCReceiveNonEmpty);
    }

    if (common::get_bit<14>(value)) {
        tx.error = false;
    }
}

} // namespace core