#include "common/logger.h"
#include "core/hardware/irq.h"

namespace core {

IRQ::IRQ(std::unique_ptr<arm::CPU>& cpu) : cpu(cpu) {}

void IRQ::reset() {
    ime = 0;
    ie = 0;
    irf = 0;
}

void IRQ::raise(IRQ::Source source) {
    irf |= 1 << static_cast<int>(source);
    logger.debug("IRQ: raise interrupt source %d", static_cast<int>(source));
    cpu->update_irq(ime && (ie & irf));
}

void IRQ::write_ime(u32 value) {
    ime = value & 0x1;
}

void IRQ::write_ie(u32 value) {
    ie = value;
}

void IRQ::write_irf(u32 value) {
    irf &= ~value;
}

} // namespace core