#pragma once

#include <vector>
#include <memory>
#include <variant>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/types.h"
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
        IRMove,
        IRLoadGPR,
        IRStoreGPR,
        IRAdd,
        IRLogicalShiftLeft,
        IRAnd,
        IRLogicalShiftRight,
        IRMemoryWrite,
        IRSub,
        IRSetFlags,
        IRStoreFlags,
        IRCompare
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
    void update_flag(Flags to_update, bool value);

    void handle_set_carry(IROpcodeVariant& opcode_variant);
    void handle_clear_carry(IROpcodeVariant& opcode_variant);
    void handle_move(IROpcodeVariant& opcode_variant);
    void handle_load_gpr(IROpcodeVariant& opcode_variant);
    void handle_store_gpr(IROpcodeVariant& opcode_variant);
    void handle_add(IROpcodeVariant& opcode_variant);
    void handle_logical_shift_left(IROpcodeVariant& opcode_variant);
    void handle_and(IROpcodeVariant& opcode_variant);
    void handle_logical_shift_right(IROpcodeVariant& opcode_variant);
    void handle_memory_write(IROpcodeVariant& opcode_variant);
    void handle_sub(IROpcodeVariant& opcode_variant);
    void handle_set_flags(IROpcodeVariant& opcode_variant);
    void handle_store_flags(IROpcodeVariant& opcode_variant);
    void handle_compare(IROpcodeVariant& opcode_variant);

    CodeCache<CompiledBlock> code_cache;
    std::vector<u32> variables;
    Flags flags{0};
    Jit& jit;
};

} // namespace arm