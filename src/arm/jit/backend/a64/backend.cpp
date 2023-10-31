#include "arm/jit/backend/a64/backend.h"
#include "arm/jit/jit.h"

namespace arm {

A64Backend::A64Backend(Jit& jit) : code_block(CODE_CACHE_SIZE), assembler(code_block.get_code()), jit(jit) {}

void A64Backend::reset() {
    code_cache.reset();
}

bool A64Backend::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void A64Backend::compile(BasicBlock& basic_block) {
    register_allocator.reset();

    // calculate the lifetimes of ir variables
    register_allocator.record_lifetimes(basic_block);

    JitFunction jit_fn = assembler.get_current_code<JitFunction>();
    code_block.unprotect();

    compile_prologue();

    Label label_pass;
    Label label_fail;

    compile_condition_check(basic_block.condition, label_pass, label_fail);

    assembler.link(label_pass);

    if (basic_block.condition != Condition::NV) {
        for (auto& opcode : basic_block.opcodes) {
            compile_ir_opcode(opcode);
            register_allocator.advance();
        }
    }

    assembler.link(label_fail);
    assembler.sub(cycles_left_reg, cycles_left_reg, basic_block.cycles);

    compile_epilogue();

    code_block.protect();

    assembler.dump();
    
    code_cache.set(basic_block.location, std::move(jit_fn));
}

int A64Backend::run(Location location, int cycles_left) {
    JitFunction jit_fn = code_cache.get(location);
    return jit_fn(&jit.state, cycles_left);
}

void A64Backend::compile_prologue() {
    // save non-volatile registers to the stack
    assembler.stp(x19, x20, sp, IndexMode::Pre, -96);
    assembler.stp(x21, x22, sp, 16);
    assembler.stp(x23, x24, sp, 32);
    assembler.stp(x25, x26, sp, 48);
    assembler.stp(x27, x28, sp, 64);
    assembler.stp(x29, x30, sp, 80);

    // store the cpu state pointer into the pinned register
    assembler.mov(state_reg, x0);

    // store the cycles left into the cycles left pinned register
    assembler.mov(cycles_left_reg, w1);

    // TODO: eventually check interrupts in jitted code to reduce time outside of jit context
}

void A64Backend::compile_epilogue() {
    // store the cycles left into w0
    assembler.mov(w0, cycles_left_reg);

    // restore non-volatile registers from the stack
    assembler.ldp(x29, x30, sp, 80);
    assembler.ldp(x27, x28, sp, 64);
    assembler.ldp(x25, x26, sp, 48);
    assembler.ldp(x23, x24, sp, 32);
    assembler.ldp(x21, x22, sp, 16);
    assembler.ldp(x19, x20, sp, IndexMode::Post, 96);

    assembler.ret();
}

void A64Backend::compile_condition_check(Condition condition, Label& label_pass, Label& label_fail) {
    if (condition != Condition::AL && condition != Condition::NV) {
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.ldr(tmp_reg, state_reg, jit.get_offset_to_cpsr());

        WReg tmp_mask_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_mask_reg, 0xf0000000);

        assembler._and(tmp_reg, tmp_reg, tmp_mask_reg);

        assembler.msr(SystemReg::NZCV, XReg{tmp_reg.id});

        assembler.b(condition, label_pass);
        assembler.b(label_fail);

        register_allocator.free_temporaries();
    }
}

void A64Backend::compile_ir_opcode(std::unique_ptr<IROpcode>& opcode) {
    auto type = opcode->get_type();
    switch (type) {
    case IROpcodeType::LoadGPR:
        compile_load_gpr(*opcode->as<IRLoadGPR>());
        break;
    case IROpcodeType::StoreGPR:
        compile_store_gpr(*opcode->as<IRStoreGPR>());
        break;
    case IROpcodeType::LoadCPSR:
        compile_load_cpsr(*opcode->as<IRLoadCPSR>());
        break;
    case IROpcodeType::StoreCPSR:
        logger.todo("handle StoreCPSR");
        break;
    case IROpcodeType::LoadSPSR:
        logger.todo("handle LoadSPSR");
        break;
    case IROpcodeType::StoreSPSR:
        logger.todo("handle StoreSPSR");
        break;
    case IROpcodeType::LoadCoprocessor:
        logger.todo("handle LoadCoprocessor");
        break;
    case IROpcodeType::StoreCoprocessor:
        logger.todo("handle StoreCoprocessor");
        break;
    case IROpcodeType::BitwiseAnd:
        compile_bitwise_and(*opcode->as<IRBitwiseAnd>());
        break;
    case IROpcodeType::BitwiseOr:
        logger.todo("handle BitwiseOr");
        break;
    case IROpcodeType::BitwiseNot:
        logger.todo("handle BitwiseNot");
        break;
    case IROpcodeType::BitwiseExclusiveOr:
        logger.todo("handle BitwiseExclusiveOr");
        break;
    case IROpcodeType::Add:
        logger.todo("handle Add");
        break;
    case IROpcodeType::AddLong:
        logger.todo("handle AddLong");
        break;
    case IROpcodeType::Subtract:
        logger.todo("handle Subtract");
        break;
    case IROpcodeType::Multiply:
        logger.todo("handle Multiply");
        break;
    case IROpcodeType::MultiplyLong:
        logger.todo("handle MultiplyLong");
        break;
    case IROpcodeType::LogicalShiftLeft:
        logger.todo("handle LogicalShiftLeft");
        break;
    case IROpcodeType::LogicalShiftRight:
        logger.todo("handle LogicalShiftRight");
        break;
    case IROpcodeType::ArithmeticShiftRight:
        logger.todo("handle ArithmeticShiftRight");
        break;
    case IROpcodeType::BarrelShifterLogicalShiftLeft:
        logger.todo("handle BarrelShifterLogicalShiftLeft");
        break;
    case IROpcodeType::BarrelShifterLogicalShiftRight:
        logger.todo("handle BarrelShifterLogicalShiftRight");
        break;
    case IROpcodeType::BarrelShifterArithmeticShiftRight:
        logger.todo("handle BarrelShifterArithmeticShiftRight");
        break;
    case IROpcodeType::BarrelShifterRotateRight:
        logger.todo("handle BarrelShifterRotateRight");
        break;
    case IROpcodeType::BarrelShifterRotateRightExtended:
        logger.todo("handle BarrelShifterRotateRightExtended");
        break;
    case IROpcodeType::CountLeadingZeroes:
        logger.todo("handle CountLeadingZeroes");
        break;
    case IROpcodeType::Compare:
        logger.todo("handle Compare");
        break;
    case IROpcodeType::Copy:
        compile_copy(*opcode->as<IRCopy>());
        break;
    case IROpcodeType::MemoryWrite:
        logger.todo("handle MemoryWrite");
        break;
    case IROpcodeType::MemoryRead:
        logger.todo("handle MemoryRead");
        break;
    }
}

void A64Backend::compile_load_gpr(IRLoadGPR& opcode) {
    u64 gpr_offset = jit.get_offset_to_gpr(opcode.src.gpr, opcode.src.mode);
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    assembler.ldr(dst_reg, state_reg, gpr_offset);
}

void A64Backend::compile_store_gpr(IRStoreGPR& opcode) {
    u64 gpr_offset = jit.get_offset_to_gpr(opcode.dst.gpr, opcode.dst.mode);

    if (opcode.src.is_constant()) {
        auto& src = opcode.src.as_constant();
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_reg, src.value);
        assembler.str(tmp_reg, state_reg, gpr_offset);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.str(src_reg, state_reg, gpr_offset);
    }
}

void A64Backend::compile_load_cpsr(IRLoadCPSR& opcode) {
    u64 cpsr_offset = jit.get_offset_to_cpsr();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    assembler.ldr(dst_reg, state_reg, cpsr_offset);
}

void A64Backend::compile_bitwise_and(IRBitwiseAnd& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    if (opcode.lhs.is_constant() && opcode.rhs.is_constant()) {
        u32 result = opcode.lhs.as_constant().value & opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else {
        logger.todo("handle bitwise where lhs and rhs aren't constants");
    }
}

void A64Backend::compile_copy(IRCopy& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    if (opcode.src.is_constant()) {
        auto& src = opcode.src.as_constant();
        assembler.mov(dst_reg, src.value);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.mov(dst_reg, src_reg);
    }
}

} // namespace arm