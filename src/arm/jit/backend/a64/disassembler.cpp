#include <llvm-c/Disassembler.h>
#include <llvm-c/Target.h>
#include "common/string.h"
#include "common/logger.h"
#include "arm/jit/backend/a64/disassembler.h"

namespace arm {

std::string disassemble_a64_instruction(u64 pc, u32 instruction) {
    std::string result;

    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64Disassembler();
    LLVMDisasmContextRef llvm_ctx = LLVMCreateDisasm("aarch64", nullptr, 0, nullptr, nullptr);
    LLVMSetDisasmOptions(llvm_ctx, LLVMDisassembler_Option_AsmPrinterVariant);

    char buffer[160];
    u64 inst_size = LLVMDisasmInstruction(llvm_ctx, reinterpret_cast<u8*>(&instruction), sizeof(instruction), pc, buffer, sizeof(buffer));
    result = inst_size > 0 ? common::format("  %016lx %08x %s", pc, instruction, buffer) : "<invalid>";

    LLVMDisasmDispose(llvm_ctx);
    return result;
}

} // namespace arm