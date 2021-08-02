#include <core/hw/interrupt/interrupt.h>
#include <core/hw/hw.h>

Interrupt::Interrupt(HW* hw, int arch) : hw(hw), arch(arch) {

}

void Interrupt::Reset() {
    IME = 0;
    IE = 0;
    IF = 0;
}