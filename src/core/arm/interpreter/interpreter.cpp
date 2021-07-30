#include <core/arm/interpreter/interpreter.h>

Interpreter::Interpreter(MemoryBase& memory, CPUArch arch) : memory(memory), arch(arch) {

}

Interpreter::~Interpreter() {
    
}