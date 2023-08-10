#include "common/bits.h"
#include "common/logger.h"
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

u32 IPC::read_ipcfiforecv(arm::Arch arch) {
    auto& tx_cnt = ipcfifocnt[static_cast<int>(arch)];
    auto& tx_recv = ipcfiforecv[static_cast<int>(arch)];
    auto& rx_cnt = ipcfifocnt[!static_cast<int>(arch)];
    auto& rx_fifo = fifo[!static_cast<int>(arch)];
    auto rx_irq = irq[!static_cast<int>(arch)];
    
    if (!rx_fifo.is_empty()) {
        tx_recv = rx_fifo.get_front();

        if (tx_cnt.enable_fifos) {
            rx_fifo.pop();

            if (rx_fifo.is_empty()) {
                rx_cnt.send_fifo_empty = true;
                tx_cnt.receive_fifo_empty = true;
                
                if (rx_cnt.send_fifo_empty_irq) {
                    rx_irq->raise(IRQ::Source::IPCSendEmpty);
                }
            } else if (rx_fifo.get_size() == 15) {
                rx_cnt.send_fifo_full = false;
                tx_cnt.receive_fifo_full = false;
            }
        }
    } else {
        tx_cnt.error = true;
    }

    return tx_recv;
}

void IPC::write_ipcsync(arm::Arch arch, u16 value, u32 mask) {
    auto& tx_sync = ipcsync[static_cast<int>(arch)];
    auto& rx_sync = ipcsync[!static_cast<int>(arch)];
    auto rx_irq = irq[!static_cast<int>(arch)];

    mask &= 0x6f00;
    tx_sync.data = (tx_sync.data & ~mask) | (value & mask);
    rx_sync.input = tx_sync.output;

    if (tx_sync.send_irq && rx_sync.enable_irq) {
        rx_irq->raise(IRQ::Source::IPCSync);
    }
}

void IPC::write_ipcfifocnt(arm::Arch arch, u16 value, u32 mask) {
    auto& tx_cnt = ipcfifocnt[static_cast<int>(arch)];
    auto& tx_fifo = fifo[static_cast<int>(arch)];
    auto tx_irq = irq[static_cast<int>(arch)];
    auto& rx_cnt = ipcfifocnt[!static_cast<int>(arch)];
    bool send_fifo_empty_irq_old = tx_cnt.send_fifo_empty_irq;
    bool receive_fifo_empty_irq_old = tx_cnt.receive_fifo_empty_irq;

    mask &= 0x8404;
    tx_cnt.data = (tx_cnt.data & ~mask) | (value & mask);

    if (common::get_bit<3>(value)) {
        tx_fifo.reset();
        tx_cnt.send_fifo_empty = true;
        tx_cnt.send_fifo_full = false;
        rx_cnt.receive_fifo_empty = true;
        rx_cnt.receive_fifo_full = false;

        if (tx_cnt.send_fifo_empty_irq) {
            tx_irq->raise(IRQ::Source::IPCSendEmpty);
        }
    }

    if (!send_fifo_empty_irq_old && tx_cnt.send_fifo_empty_irq && tx_cnt.send_fifo_empty) {
        tx_irq->raise(IRQ::Source::IPCSendEmpty);
    }

    if (!receive_fifo_empty_irq_old && tx_cnt.receive_fifo_empty_irq && tx_cnt.receive_fifo_empty) {
        tx_irq->raise(IRQ::Source::IPCReceiveNonEmpty);
    }

    if (common::get_bit<14>(value)) {
        tx_cnt.error = false;
    }
}

void IPC::write_ipcfifosend(arm::Arch arch, u32 value) {
    auto& tx_cnt = ipcfifocnt[static_cast<int>(arch)];
    auto& tx_fifo = fifo[static_cast<int>(arch)];
    auto& rx_cnt = ipcfifocnt[!static_cast<int>(arch)];
    auto rx_irq = irq[!static_cast<int>(arch)];
    
    if (tx_cnt.enable_fifos) {
        if (tx_fifo.get_size() < 16) {
            tx_fifo.push(value);

            if (tx_fifo.get_size() == 1) {
                tx_cnt.send_fifo_empty = false;
                rx_cnt.receive_fifo_empty = false;

                if (rx_cnt.receive_fifo_empty_irq) {
                    rx_irq->raise(IRQ::Source::IPCReceiveNonEmpty);
                }
            } else if (tx_fifo.get_size() == 16) {
                tx_cnt.send_fifo_full = true;
                rx_cnt.receive_fifo_full = true;
            }
        } else {
            tx_cnt.error = true;
        }
    }
}

} // namespace core