#include <core/arm/cpu_base.h>

CPUBase::CPUBase(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {

}

void CPUBase::SendInterrupt(int interrupt) {
    // set the appropriate bit in IF
    irf |= (1 << interrupt);
    
    // check if the interrupt is enabled too
    if (ie & (1 << interrupt)) {
        // to unhalt on the arm9 ime needs to be set
        if (ime || arch == CPUArch::ARMv4) {
            halted = false;
        }
    }
}

void CPUBase::Halt() {
    halted = true;
}

auto CPUBase::Halted() -> bool {
    return halted;
}