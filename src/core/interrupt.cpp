#include <core/core.h>
#include <core/interrupt.h>

Interrupt::Interrupt(Core* core, int arch) : core(core), arch(arch) {

}

void Interrupt::Reset() {
    IME = 0;
    IE = 0;
    IF = 0;
}