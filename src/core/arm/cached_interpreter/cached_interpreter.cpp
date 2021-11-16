#include <core/arm/cached_interpreter/cached_interpreter.h>
#include <core/hw/cp15/cp15.h>

CachedInterpreter::CachedInterpreter(MemoryBase& memory, CPUArch arch, CP15* cp15) : CPUBase(memory, arch), cp15(cp15) {

}

CachedInterpreter::~CachedInterpreter() {}

void CachedInterpreter::Reset() {
    
}

void CachedInterpreter::Run(int cycles) {
    
}

void CachedInterpreter::DirectBoot(u32 entrypoint) {
    
}

void CachedInterpreter::FirmwareBoot() {
    
}