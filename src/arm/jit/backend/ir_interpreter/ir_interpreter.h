#pragma once

#include <vector>
#include <memory>
#include <variant>
#include "arm/jit/basic_block.h"
#include "arm/jit/backend/backend.h"
#include "arm/jit/backend/code_cache.h"

namespace arm {

class IRInterpreter : public Backend {
public:
    virtual void reset() override;
    virtual bool has_code_at(Location location) override;
    virtual void compile(BasicBlock& basic_block) override;
    virtual void run(Location location) override;

private:
    using IROpcodeVariant = std::variant<
        IRSetCarry,
        IRClearCarry,
        IRMove,
        IRLoadGPR,
        IRStoreGPR,
        IRAdd
    >;

    using Function = void (IRInterpreter::*)(IROpcodeVariant& opcode);

    struct CompiledInstruction {
        Function fn;
        IROpcodeVariant opcode_variant;
    };

    CompiledInstruction compile_ir_opcode(std::unique_ptr<IROpcode>& opcode);

    void handle_set_carry(IROpcodeVariant& opcode_variant);
    void handle_clear_carry(IROpcodeVariant& opcode_variant);
    void handle_move(IROpcodeVariant& opcode_variant);
    void handle_load_gpr(IROpcodeVariant& opcode_variant);
    void handle_store_gpr(IROpcodeVariant& opcode_variant);
    void handle_add(IROpcodeVariant& opcode_variant);

    CodeCache<std::vector<CompiledInstruction>> code_cache;
    std::vector<u32> variables;
};

} // namespace arm