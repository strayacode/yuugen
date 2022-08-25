#include "Core/ARM/CPU.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM7/Coprocessor.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/ARM/ARM9/Coprocessor.h"
#include "Core/ARM/Interpreter/Interpreter.h"
#include "Core/System.h"

CPU::CPU(System& system, Arch arch) {
    if (arch == Arch::ARMv5) {
        m_memory = std::make_unique<ARM9Memory>(system);
        m_coprocessor = std::make_unique<ARM9Coprocessor>(*this); 
    } else {
        m_memory = std::make_unique<ARM7Memory>(system);
        m_coprocessor = std::make_unique<ARM7Coprocessor>();
    }
}

void CPU::reset() {

}

void CPU::direct_boot() {

}

void CPU::firmware_boot() {

}

void CPU::run(u64 target) {
    m_executor->run(target);
}

void CPU::select_executor(ExecutorType executor_type) {
    switch (executor_type) {
    case ExecutorType::Interpreter:
        m_executor = std::make_unique<Interpreter>(*this);
        break;
    default:
        log_fatal("[CPU] handle unknown executor");
    }
}
