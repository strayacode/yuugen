#include "Core/ARM/CPU.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/System.h"

CPU::CPU(System& system, Arch arch) {
    if (arch == Arch::ARMv5) {
        memory = ARM9Memory(system);
        coprocessor = ARM9Coprocessor(system); 
    } else {
        memory = ARM7Memory(system);
        coprocessor = ARM7Coprocessor(system);
    }
}

void CPU::reset() {

}

void CPU::direct_boot() {

}

void CPU::firmware_boot() {

}

void CPU::run(u64 target) {
    executor->run(target);
}

void CPU::select_executor(ExecutorType executor_type) {
    switch (executor_type) {
    case ExecutorType::Interpreter:
        executor = std::make_unique<Interpreter>(*this);
        break;
    default:
        log_fatal("[CPU] handle unknown executor");
    }
}
