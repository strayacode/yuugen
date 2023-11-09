#pragma once

#include "common/logger.h"
#include "arm/state.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/backend/backend.h"
#include "arm/jit/backend/code_cache.h"
#include "arm/jit/backend/a64/code_block.h"
#include "arm/jit/backend/a64/assembler.h"
#include "arm/jit/backend/a64/register_allocator.h"

namespace arm {

class Jit;

class A64Backend : public Backend {
public:
    A64Backend(Jit& jit);

    void reset() override;
    Code get_code_at(Location location) override;
    Code compile(BasicBlock& basic_block) override;
    int run(Code code, int cycles_left) override;

private:
    // return value: the cycles left after running the jit function (w0)
    // argument 1: a 64-bit pointer to the Jit class (x0)
    // argument 2: the cycles left (w1)
    using JitFunction = int (*)(Jit* jit, int cycles_left);

    void push_volatile_registers();
    void pop_volatile_registers();

    void compile_prologue();
    void compile_epilogue();
    void compile_condition_check(BasicBlock& basic_block, Label& label_pass, Label& label_fail);

    void compile_ir_opcode(std::unique_ptr<IROpcode>& opcode);
    void compile_load_gpr(IRLoadGPR& opcode);
    void compile_store_gpr(IRStoreGPR& opcode);
    void compile_load_cpsr(IRLoadCPSR& opcode);
    void compile_store_cpsr(IRStoreCPSR& opcode);
    void compile_load_spsr(IRLoadSPSR& opcode);
    void compile_store_spsr(IRStoreSPSR& opcode);
    void compile_bitwise_and(IRBitwiseAnd& opcode);
    void compile_bitwise_or(IRBitwiseOr& opcode);
    void compile_bitwise_not(IRBitwiseNot& opcode);
    void compile_bitwise_exclusive_or(IRBitwiseExclusiveOr& opcode);
    void compile_add(IRAdd& opcode);
    void compile_subtract(IRSubtract& opcode);
    void compile_multiply(IRMultiply& opcode);
    void compile_logical_shift_left(IRLogicalShiftLeft& opcode);
    void compile_logical_shift_right(IRLogicalShiftRight& opcode);
    void compile_barrel_shifter_logical_shift_left(IRBarrelShifterLogicalShiftLeft& opcode);
    void compile_barrel_shifter_logical_shift_right(IRBarrelShifterLogicalShiftRight& opcode);
    void compile_compare(IRCompare& opcode);
    void compile_copy(IRCopy& opcode);
    void compile_get_bit(IRGetBit& opcode);
    void compile_set_bit(IRSetBit& opcode);
    void compile_memory_read(IRMemoryRead& opcode);
    void compile_memory_write(IRMemoryWrite& opcode);

    CodeCache<JitFunction> code_cache;
    CodeBlock code_block;
    A64Assembler assembler;
    Jit& jit;

    static constexpr int CODE_CACHE_SIZE = 16 * 1024 * 1024;

    static constexpr XReg jit_reg = x19;
    static constexpr WReg cycles_left_reg = w20;

    // we have w21, w22, w23, w24, w25, w26, w27 and w28 available for register allocation
    RegisterAllocator register_allocator;
};

} // namespace arm