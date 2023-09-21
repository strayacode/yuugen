#pragma once

#include <vector>
#include <memory>
#include <variant>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/backend/backend.h"
#include "arm/jit/backend/code_cache.h"

namespace arm {

class Jit;

class IRInterpreter : public Backend {
public:
    IRInterpreter(Jit& jit);

    virtual void reset() override;
    virtual bool has_code_at(Location location) override;
    virtual void compile(BasicBlock& basic_block) override;
    virtual int run(Location location) override;

private:
    bool evaluate_condition(Condition condition);

    using IROpcodeVariant = std::variant<
        // state opcodes
        IRLoadGPR,
        IRStoreGPR,
        IRLoadCPSR,
        IRStoreCPSR,
        IRLoadSPSR,
        IRStoreSPSR,

        // bitwise opcodes
        IRBitwiseAnd,
        IRBitwiseOr,
        IRBitwiseNot,
        IRBitwiseExclusiveOr,

        // flag opcodes
        IRCompare,

        // branch opcodes
        IRBranch,
        IRBranchExchange,

        // misc opcodes
        IRCopy,

        IRAdd,
        IRLogicalShiftLeft,
        IRLogicalShiftRight,
        IRMemoryWrite,
        IRSub,
        IRArithmeticShiftRight,
        IRRotateRight,
        IRMemoryRead,
        IRMultiply,
        IRAddCarry
    >;

    using Function = void (IRInterpreter::*)(IROpcodeVariant& opcode);

    struct CompiledInstruction {
        Function fn;
        IROpcodeVariant opcode_variant;
    };

    struct CompiledBlock {
        int cycles;
        int num_instructions;
        Condition condition;
        Location location;
        std::vector<CompiledInstruction> instructions;
    };

    CompiledInstruction compile_ir_opcode(std::unique_ptr<IROpcode>& opcode);

    u32& get(IRVariable& variable);
    u32& get_or_allocate(IRVariable& variable);
    void assign_variable(IRVariable& variable, u32 value);
    u32 resolve_value(IRValue& value);
    void dump_variables();

    // state opcodes
    void handle_load_gpr(IROpcodeVariant& opcode_variant);
    void handle_store_gpr(IROpcodeVariant& opcode_variant);
    void handle_load_cpsr(IROpcodeVariant& opcode_variant);
    void handle_store_cpsr(IROpcodeVariant& opcode_variant);
    void handle_load_spsr(IROpcodeVariant& opcode_variant);
    void handle_store_spsr(IROpcodeVariant& opcode_variant);

    // bitwise opcodes
    void handle_bitwise_and(IROpcodeVariant& opcode_variant);
    void handle_bitwise_or(IROpcodeVariant& opcode_variant);
    void handle_bitwise_not(IROpcodeVariant& opcode_variant);
    void handle_bitwise_exclusive_or(IROpcodeVariant& opcode_variant);

    // arithmetic opcodes
    void handle_add(IROpcodeVariant& opcode_variant);
    void handle_sub(IROpcodeVariant& opcode_variant);

    // flag opcodes
    void handle_compare(IROpcodeVariant& opcode_variant);

    // branch opcodes
    void handle_branch(IROpcodeVariant& opcode_variant);
    void handle_branch_exchange(IROpcodeVariant& opcode_variant);

    // misc opcodes
    void handle_copy(IROpcodeVariant& opcode_variant);

    void handle_set_carry(IROpcodeVariant& opcode_variant);
    void handle_clear_carry(IROpcodeVariant& opcode_variant);
    
    void handle_logical_shift_left(IROpcodeVariant& opcode_variant);
    void handle_logical_shift_right(IROpcodeVariant& opcode_variant);
    void handle_memory_write(IROpcodeVariant& opcode_variant);
    void handle_arithmetic_shift_right(IROpcodeVariant& opcode_variant);
    void handle_rotate_right(IROpcodeVariant& opcode_variant);
    void handle_memory_read(IROpcodeVariant& opcode_variant);
    void handle_multiply(IROpcodeVariant& opcode_variant);
    void handle_add_carry(IROpcodeVariant& opcode_variant);

    CodeCache<CompiledBlock> code_cache;
    std::vector<u32> variables;
    Jit& jit;
};

} // namespace arm