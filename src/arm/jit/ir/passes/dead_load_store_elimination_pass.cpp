#include "arm/jit/ir/passes/dead_load_store_elimination_pass.h"

namespace arm {

void DeadLoadStoreEliminationPass::optimise(BasicBlock& basic_block) {
    gpr_uses.fill(IRValue{});
    cpsr_use = IRValue{};
    spsr_use = IRValue{};

    // convert loads to copies with last known value
    for (auto& opcode : basic_block.opcodes) {
        if (opcode->get_type() == IROpcodeType::LoadGPR) {
            auto load_gpr_opcode = *opcode->as<IRLoadGPR>();
            if (gpr_uses[load_gpr_opcode.src.get_id()].is_assigned()) {
                opcode = std::make_unique<IRCopy>(load_gpr_opcode.dst, gpr_uses[load_gpr_opcode.src.get_id()]);
                mark_modified();
            } else {
                gpr_uses[load_gpr_opcode.src.get_id()] = load_gpr_opcode.dst;
            }
        } else if (opcode->get_type() == IROpcodeType::StoreGPR) {
            auto store_gpr_opcode = *opcode->as<IRStoreGPR>();
            gpr_uses[store_gpr_opcode.dst.get_id()] = store_gpr_opcode.src;
        } else if (opcode->get_type() == IROpcodeType::LoadCPSR) {
            auto load_cpsr_opcode = *opcode->as<IRLoadCPSR>();
            if (cpsr_use.is_assigned()) {
                opcode = std::make_unique<IRCopy>(load_cpsr_opcode.dst, cpsr_use);
                mark_modified();
            } else {
                cpsr_use = load_cpsr_opcode.dst;
            }
        } else if (opcode->get_type() == IROpcodeType::StoreCPSR) {
            auto store_cpsr_opcode = *opcode->as<IRStoreCPSR>();
            cpsr_use = store_cpsr_opcode.src;
        } else if (opcode->get_type() == IROpcodeType::LoadSPSR) {
            auto load_spsr_opcode = *opcode->as<IRLoadSPSR>();
            if (spsr_use.is_assigned()) {
                opcode = std::make_unique<IRCopy>(load_spsr_opcode.dst, spsr_use);
                mark_modified();
            } else {
                spsr_use = load_spsr_opcode.dst;
            }
        } else if (opcode->get_type() == IROpcodeType::StoreSPSR) {
            auto store_spsr_opcode = *opcode->as<IRStoreSPSR>();
            spsr_use = store_spsr_opcode.src;
        }
    }

    gpr_uses.fill(IRValue{});
    cpsr_use = IRValue{};
    spsr_use = IRValue{};
    
    // remove all stores which aren't the last store
    auto it = basic_block.opcodes.rbegin();
    while (it != basic_block.opcodes.rend()) {
        auto& opcode = *it;
        if (opcode->get_type() == IROpcodeType::StoreGPR) {
            auto store_gpr_opcode = *opcode->as<IRStoreGPR>();
            if (gpr_uses[store_gpr_opcode.dst.get_id()].is_assigned()) {
                it = std::reverse_iterator(basic_block.opcodes.erase(std::next(it).base()));
                mark_modified();
            } else {
                gpr_uses[store_gpr_opcode.dst.get_id()] = store_gpr_opcode.src;
                it++;
            }
        } else if (opcode->get_type() == IROpcodeType::StoreCPSR) {
            auto store_cpsr_opcode = *opcode->as<IRStoreCPSR>();
            if (cpsr_use.is_assigned()) {
                it = std::reverse_iterator(basic_block.opcodes.erase(std::next(it).base()));
                mark_modified();
            } else {
                cpsr_use = store_cpsr_opcode.src;
                it++;
            }
        } else if (opcode->get_type() == IROpcodeType::StoreSPSR) {
            auto store_spsr_opcode = *opcode->as<IRStoreSPSR>();
            if (spsr_use.is_assigned()) {
                it = std::reverse_iterator(basic_block.opcodes.erase(std::next(it).base()));
                mark_modified();
            } else {
                spsr_use = store_spsr_opcode.src;
                it++;
            }
        } else {
            it++;
        }
    }
}

} // namespace arm