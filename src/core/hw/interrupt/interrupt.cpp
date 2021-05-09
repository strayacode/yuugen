#include <core/hw/interrupt/interrupt.h>
#include <core/core.h>

Interrupt::Interrupt(Core* core, int arch) : core(core), arch(arch) {

}

void Interrupt::Reset() {
    IME = 0;
    IE = 0;
    IF = 0;
}