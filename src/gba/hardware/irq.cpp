#include "common/logger.h"
#include "gba/hardware/irq.h"

namespace gba {

IRQ::IRQ(std::unique_ptr<arm::CPU>& cpu) : cpu(cpu) {}

void IRQ::reset() {
    ime = 0;
    ie = 0;
    irf = 0;
}

void IRQ::raise(IRQ::Source source) {
    irf |= 1 << static_cast<int>(source);

    if (ime && (ie & (1 << static_cast<int>(source)))) {
        cpu->update_halted(false);
    }

    cpu->update_irq(ime && (ie & irf));
}

void IRQ::write_ime(u16 value, u32 mask) {
    mask &= 0x1;
    ime = (ime & ~mask) | (value & mask);

    cpu->update_irq(ime && (ie & irf));
}

void IRQ::write_ie(u16 value, u32 mask) {
    ie = (ie & ~mask) | (value & mask);
    cpu->update_irq(ime && (ie & irf));
}

void IRQ::write_irf(u16 value, u32 mask) {
    irf &= ~(value & mask);
    cpu->update_irq(ime && (ie & irf));
}

} // namespace gba