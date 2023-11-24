#include "arm/arithmetic.h"
#include "arm/jit/backend/a64/backend.h"
#include "arm/jit/jit.h"

namespace arm {

A64Backend::A64Backend(Jit& jit) : code_block(CODE_CACHE_SIZE), assembler(code_block.get_code()), jit(jit) {}

// TODO: put memory stuff in separate file
u8 read_byte(Jit* jit, u32 addr) {
    return jit->read_byte(addr);
}

u16 read_half(Jit* jit, u32 addr) {
    return jit->read_half(addr);
}

u32 read_word(Jit* jit, u32 addr) {
    return jit->read_word(addr);
}

u32 read_word_rotate(Jit* jit, u32 addr) {
    return jit->read_word_rotate(addr);
}

void write_byte(Jit* jit, u32 addr, u8 data) {
    jit->write_byte(addr, data);
}

void write_half(Jit* jit, u32 addr, u16 data) {
    jit->write_half(addr, data);
}

void write_word(Jit* jit, u32 addr, u32 data) {
    jit->write_word(addr, data);
}

void A64Backend::reset() {
    code_cache.reset();
}

Code A64Backend::get_code_at(Location location) {
    return reinterpret_cast<void*>(code_cache.get_or_create(location));
}

Code A64Backend::compile(BasicBlock& basic_block) {
    logger.print("compiling basic block at %08x...", basic_block.location.get_address());
    register_allocator.reset();

    // calculate the lifetimes of ir variables
    register_allocator.record_lifetimes(basic_block);

    JitFunction jit_fn = assembler.get_current_code<JitFunction>();
    code_block.unprotect();

    logger.print("compiling prologue...");
    compile_prologue();

    Label label_pass;
    Label label_fail;

    logger.print("compiling condition check...");
    compile_condition_check(basic_block, label_pass, label_fail);
    assembler.link(label_pass);

    if (basic_block.condition != Condition::NV) {
        for (auto& opcode : basic_block.opcodes) {
            logger.print("compiling %s...", opcode->to_string().c_str());
            compile_ir_opcode(opcode);
            register_allocator.advance();
        }
    }

    assembler.link(label_fail);

    logger.print("compiling epilogue...");
    assembler.sub(cycles_left_reg, cycles_left_reg, static_cast<u64>(basic_block.cycles));
    compile_epilogue();

    logger.print("");
    assembler.dump();

    code_block.protect();
    code_cache.set(basic_block.location, jit_fn);
    return reinterpret_cast<void*>(jit_fn);
}

int A64Backend::run(Code code, int cycles_left) {
    JitFunction jit_fn = reinterpret_cast<JitFunction>(code);
    return jit_fn(&jit, cycles_left);
}

void A64Backend::push_volatile_registers() {
    assembler.stp(x8, x9, sp, IndexMode::Pre, -64);
    assembler.stp(x10, x11, sp, 16);
    assembler.stp(x12, x13, sp, 32);
    assembler.stp(x14, x15, sp, 48);
}

void A64Backend::pop_volatile_registers() {
    assembler.ldp(x8, x9, sp, 48);
    assembler.ldp(x10, x11, sp, 32);
    assembler.ldp(x12, x13, sp, 16);
    assembler.ldp(x14, x15, sp, IndexMode::Post, 64);
}

void A64Backend::compile_prologue() {
    // save non-volatile registers to the stack
    assembler.stp(x19, x20, sp, IndexMode::Pre, -96);
    assembler.stp(x21, x22, sp, 16);
    assembler.stp(x23, x24, sp, 32);
    assembler.stp(x25, x26, sp, 48);
    assembler.stp(x27, x28, sp, 64);
    assembler.stp(x29, x30, sp, 80);

    // store the jit pointer into the pinned register
    assembler.mov(jit_reg, x0);

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

void A64Backend::compile_condition_check(BasicBlock& basic_block, Label& label_pass, Label& label_fail) {
    if (basic_block.condition != Condition::AL && basic_block.condition != Condition::NV) {
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.ldr(tmp_reg, jit_reg, jit.get_offset_to_cpsr());
        assembler._and(tmp_reg, tmp_reg, 0xf0000000);

        assembler.msr(SystemReg::NZCV, XReg{tmp_reg.id});

        assembler.b(basic_block.condition, label_pass);

        // update pc to be after block when the condition fails
        u32 pc_after_block = basic_block.location.get_address() + ((2 + basic_block.num_instructions) * basic_block.location.get_instruction_size());
        WReg tmp_pc_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_pc_reg, pc_after_block);
        assembler.str(tmp_pc_reg, jit_reg, jit.get_offset_to_gpr(GPR::PC, Mode::USR));
        
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
        compile_store_cpsr(*opcode->as<IRStoreCPSR>());
        break;
    case IROpcodeType::LoadSPSR:
        compile_load_spsr(*opcode->as<IRLoadSPSR>());
        break;
    case IROpcodeType::StoreSPSR:
        compile_store_spsr(*opcode->as<IRStoreSPSR>());
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
        compile_bitwise_or(*opcode->as<IRBitwiseOr>());
        break;
    case IROpcodeType::BitwiseNot:
        compile_bitwise_not(*opcode->as<IRBitwiseNot>());
        break;
    case IROpcodeType::BitwiseExclusiveOr:
        compile_bitwise_exclusive_or(*opcode->as<IRBitwiseExclusiveOr>());
        break;
    case IROpcodeType::Add:
        compile_add(*opcode->as<IRAdd>());
        break;
    case IROpcodeType::AddLong:
        compile_add_long(*opcode->as<IRAddLong>());
        break;
    case IROpcodeType::Subtract:
        compile_subtract(*opcode->as<IRSubtract>());
        break;
    case IROpcodeType::Multiply:
        compile_multiply(*opcode->as<IRMultiply>());
        break;
    case IROpcodeType::MultiplyLong:
        compile_multiply_long(*opcode->as<IRMultiplyLong>());
        break;
    case IROpcodeType::LogicalShiftLeft:
        compile_logical_shift_left(*opcode->as<IRLogicalShiftLeft>());
        break;
    case IROpcodeType::LogicalShiftRight:
        compile_logical_shift_right(*opcode->as<IRLogicalShiftRight>());
        break;
    case IROpcodeType::ArithmeticShiftRight:
        compile_arithmetic_shift_right(*opcode->as<IRArithmeticShiftRight>());
        break;
    case IROpcodeType::RotateRight:
        logger.todo("handle RotateRight");
        break;
    case IROpcodeType::BarrelShifterLogicalShiftLeft:
        compile_barrel_shifter_logical_shift_left(*opcode->as<IRBarrelShifterLogicalShiftLeft>());
        break;
    case IROpcodeType::BarrelShifterLogicalShiftRight:
        compile_barrel_shifter_logical_shift_right(*opcode->as<IRBarrelShifterLogicalShiftRight>());
        break;
    case IROpcodeType::BarrelShifterArithmeticShiftRight:
        compile_barrel_shifter_arithmetic_shift_right(*opcode->as<IRBarrelShifterArithmeticShiftRight>());
        break;
    case IROpcodeType::BarrelShifterRotateRight:
        logger.todo("handle BarrelShifterRotateRight");
        break;
    case IROpcodeType::BarrelShifterRotateRightExtended:
        compile_barrel_shifter_rotate_right_extended(*opcode->as<IRBarrelShifterRotateRightExtended>());
        break;
    case IROpcodeType::CountLeadingZeroes:
        logger.todo("handle CountLeadingZeroes");
        break;
    case IROpcodeType::Compare:
        compile_compare(*opcode->as<IRCompare>());
        break;
    case IROpcodeType::Copy:
        compile_copy(*opcode->as<IRCopy>());
        break;
    case IROpcodeType::GetBit:
        compile_get_bit(*opcode->as<IRGetBit>());
        break;
    case IROpcodeType::SetBit:
        compile_set_bit(*opcode->as<IRSetBit>());
        break;
    case IROpcodeType::MemoryRead:
        compile_memory_read(*opcode->as<IRMemoryRead>());
        break;
    case IROpcodeType::MemoryWrite:
        compile_memory_write(*opcode->as<IRMemoryWrite>());
        break;
    }
}

void A64Backend::compile_load_gpr(IRLoadGPR& opcode) {
    u64 gpr_offset = jit.get_offset_to_gpr(opcode.src.gpr, opcode.src.mode);
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    assembler.ldr(dst_reg, jit_reg, gpr_offset);
}

void A64Backend::compile_store_gpr(IRStoreGPR& opcode) {
    u64 gpr_offset = jit.get_offset_to_gpr(opcode.dst.gpr, opcode.dst.mode);

    if (opcode.src.is_constant()) {
        auto& src = opcode.src.as_constant();
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_reg, src.value);
        assembler.str(tmp_reg, jit_reg, gpr_offset);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.str(src_reg, jit_reg, gpr_offset);
    }
}

void A64Backend::compile_load_cpsr(IRLoadCPSR& opcode) {
    u64 cpsr_offset = jit.get_offset_to_cpsr();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    assembler.ldr(dst_reg, jit_reg, cpsr_offset);
}

void A64Backend::compile_store_cpsr(IRStoreCPSR& opcode) {
    u64 cpsr_offset = jit.get_offset_to_cpsr();

    if (opcode.src.is_constant()) {
        auto& src = opcode.src.as_constant();
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_reg, src.value);
        assembler.str(tmp_reg, jit_reg, cpsr_offset);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.str(src_reg, jit_reg, cpsr_offset);
    }
}

void A64Backend::compile_load_spsr(IRLoadSPSR& opcode) {
    u64 spsr_offset = jit.get_offset_to_spsr(opcode.mode);
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    assembler.ldr(dst_reg, jit_reg, spsr_offset);
}

void A64Backend::compile_store_spsr(IRStoreSPSR& opcode) {
    u64 spsr_offset = jit.get_offset_to_spsr(opcode.mode);

    if (opcode.src.is_constant()) {
        auto& src = opcode.src.as_constant();
        WReg tmp_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_reg, src.value);
        assembler.str(tmp_reg, jit_reg, spsr_offset);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.str(src_reg, jit_reg, spsr_offset);
    }
}

void A64Backend::compile_logical_shift_left(IRLogicalShiftLeft& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);

    if (src_is_constant && amount_is_constant) {
        u32 result = opcode.src.as_constant().value << opcode.amount.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_constant();
        WReg src_reg = register_allocator.get(src);
        assembler.lsl(dst_reg, src_reg, amount.value & 0x1f);
    } else {
        logger.todo("handle lsl case");
    }
}

void A64Backend::compile_logical_shift_right(IRLogicalShiftRight& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);

    if (src_is_constant && amount_is_constant) {
        u32 result = opcode.src.as_constant().value >> opcode.amount.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!src_is_constant && amount_is_constant) {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        const u32 amount = opcode.amount.as_constant().value & 0x1f;
        assembler.lsr(dst_reg, src_reg, amount);
    } else {
        logger.todo("handle lsr case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_arithmetic_shift_right(IRArithmeticShiftRight& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);

    if (src_is_constant && amount_is_constant) {
        u32 result = static_cast<s32>(opcode.src.as_constant().value) >> opcode.amount.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_constant();
        WReg src_reg = register_allocator.get(src);
        assembler.asr(dst_reg, src_reg, amount.value & 0x1f);
    } else {
        logger.todo("handle asr case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_barrel_shifter_logical_shift_left(IRBarrelShifterLogicalShiftLeft& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg result_reg = register_allocator.allocate(opcode.result_and_carry.first);
    WReg carry_reg = register_allocator.allocate(opcode.result_and_carry.second);
    WReg carry_in_reg = register_allocator.get(opcode.carry.as_variable());

    if (opcode.carry.is_constant()) {
        logger.todo("carry in for barrel shifter lsl being constant was not expected");
    }

    if (src_is_constant && amount_is_constant) {
        auto [result, carry] = lsl(opcode.src.as_constant().value, opcode.amount.as_constant().value);
        assembler.mov(result_reg, result);

        if (carry) {
            assembler.mov(carry_reg, static_cast<u32>(*carry));
        } else {
            assembler.mov(carry_reg, carry_in_reg);
        }
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_constant();
        WReg src_reg = register_allocator.get(src);
        
        if (amount.value == 0) {
            assembler.mov(result_reg, src_reg);
            assembler.mov(carry_reg, carry_in_reg);
        } else if (amount.value >= 32) {
            logger.todo("barrel shifter lsl handle amount >= 32");
        } else {
            assembler.lsl(result_reg, src_reg, amount.value);
            assembler.lsr(carry_reg, src_reg, 32 - amount.value);
            assembler._and(carry_reg, carry_reg, 0x1);
        }
    } else if (!src_is_constant && !amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_variable();
        WReg src_reg = register_allocator.get(src);
        WReg amount_reg = register_allocator.get(amount);

        Label label_ge32;
        Label label_else;
        Label label_finish1;
        Label label_finish2;

        assembler.cmp(amount_reg, 0);
        assembler.b(Condition::NE, label_ge32);

        // if amount == 0
        assembler.mov(result_reg, src_reg);
        assembler.mov(carry_reg, carry_in_reg);
        assembler.b(label_finish1);

        assembler.link(label_ge32);
        assembler.cmp(amount_reg, 31);
        assembler.b(Condition::LE, label_else);

        // if amount >= 32
        assembler.mov(result_reg, 0);
        assembler.cmp(amount_reg, 32);
        assembler.cset(carry_reg, Condition::EQ);

        WReg tmp_masked_src_reg = register_allocator.allocate_temporary();
        assembler._and(tmp_masked_src_reg, src_reg, 0x1);
        assembler._and(carry_reg, carry_reg, tmp_masked_src_reg);
        assembler.b(label_finish2);

        // amount > 0 && amount < 32
        assembler.link(label_else);
        assembler.lsl(result_reg, src_reg, amount_reg);

        WReg tmp_negated_amount_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_negated_amount_reg, 32);
        assembler.sub(tmp_negated_amount_reg, tmp_negated_amount_reg, amount_reg);

        assembler.lsr(carry_reg, src_reg, tmp_negated_amount_reg);
        assembler._and(carry_reg, carry_reg, 0x1);

        assembler.link(label_finish1);
        assembler.link(label_finish2);
    } else {
        logger.todo("handle barrel shifter lsl case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_barrel_shifter_logical_shift_right(IRBarrelShifterLogicalShiftRight& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg result_reg = register_allocator.allocate(opcode.result_and_carry.first);
    WReg carry_reg = register_allocator.allocate(opcode.result_and_carry.second);
    WReg carry_in_reg = register_allocator.get(opcode.carry.as_variable());

    if (opcode.carry.is_constant()) {
        logger.todo("carry in for barrel shifter lsr being constant was not expected");
    }

    if (src_is_constant && amount_is_constant) {
        auto [result, carry] = lsr(opcode.src.as_constant().value, opcode.amount.as_constant().value, opcode.imm);
        assembler.mov(result_reg, result);

        if (carry) {
            assembler.mov(carry_reg, static_cast<u32>(*carry));
        } else {
            assembler.mov(carry_reg, carry_in_reg);
        }
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        auto amount = opcode.amount.as_constant();

        if (amount.value == 0 && opcode.imm) {
            amount.value = 32;
        }

        if (amount.value == 0) {
            logger.todo("barrel shifter lsr handle amount == 0");
        } else if (amount.value >= 32) {
            assembler.mov(result_reg, 0);
            assembler.lsr(carry_reg, src_reg, 31);
            assembler._and(carry_reg, carry_reg, amount.value == 32);
        } else {
            assembler.lsr(result_reg, src_reg, amount.value);
            assembler.lsr(carry_reg, src_reg, amount.value - 1);
            assembler._and(carry_reg, carry_reg, 0x1);
        }
    } else if (!src_is_constant && !amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_variable();
        WReg src_reg = register_allocator.get(src);
        WReg amount_reg = register_allocator.get(amount);

        Label label_ge32;
        Label label_else;
        Label label_finish1;
        Label label_finish2;

        assembler.cmp(amount_reg, 0);
        assembler.b(Condition::NE, label_ge32);

        // if amount == 0
        if (opcode.imm) {
            assembler.mov(amount_reg, 32);
            assembler.b(label_ge32);
        } else {
            assembler.mov(result_reg, src_reg);
            assembler.mov(carry_reg, carry_in_reg);
            assembler.b(label_finish1);
        }

        assembler.link(label_ge32);
        assembler.cmp(amount_reg, 31);
        assembler.b(Condition::LE, label_else);

        // if amount >= 32
        assembler.mov(result_reg, 0);
        assembler.cmp(amount_reg, 32);
        assembler.cset(carry_reg, Condition::EQ);

        WReg tmp_shifted_src_reg = register_allocator.allocate_temporary();
        assembler.lsr(tmp_shifted_src_reg, src_reg, 31);
        assembler._and(carry_reg, carry_reg, tmp_shifted_src_reg);
        assembler.b(label_finish2);

        // amount > 0 && amount < 32
        assembler.link(label_else);
        assembler.lsr(result_reg, src_reg, amount_reg);

        assembler.sub(amount_reg, amount_reg, 1);

        assembler.lsr(carry_reg, src_reg, amount_reg);
        assembler._and(carry_reg, carry_reg, 0x1);

        assembler.link(label_finish1);
        assembler.link(label_finish2);
    } else {
        logger.todo("handle barrel shifter lsr case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_barrel_shifter_arithmetic_shift_right(IRBarrelShifterArithmeticShiftRight& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg result_reg = register_allocator.allocate(opcode.result_and_carry.first);
    WReg carry_reg = register_allocator.allocate(opcode.result_and_carry.second);
    WReg carry_in_reg = register_allocator.get(opcode.carry.as_variable());

    if (opcode.carry.is_constant()) {
        logger.todo("carry in for barrel shifter asr being constant was not expected");
    }

    if (src_is_constant && amount_is_constant) {
        auto [result, carry] = asr(opcode.src.as_constant().value, opcode.amount.as_constant().value, opcode.imm);
        assembler.mov(result_reg, result);

        if (carry) {
            assembler.mov(carry_reg, static_cast<u32>(*carry));
        } else {
            assembler.mov(carry_reg, carry_in_reg);
        }
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        auto amount = opcode.amount.as_constant();
        WReg src_reg = register_allocator.get(src);

        if (amount.value == 0 && opcode.imm) {
            amount.value = 32;
        }
        
        if (amount.value == 0) {
            assembler.mov(result_reg, src_reg);
            assembler.mov(carry_reg, carry_in_reg);
        } else if (amount.value >= 32) {
            assembler.asr(result_reg, src_reg, 31);
            assembler.lsr(carry_reg, src_reg, 31);
        } else {
            assembler.asr(result_reg, src_reg, amount.value);
            assembler.ubfx(carry_reg, src_reg, amount.value - 1, 1);
        }
    } else if (!src_is_constant && !amount_is_constant) {
        const auto src = opcode.src.as_variable();
        const auto amount = opcode.amount.as_variable();
        WReg src_reg = register_allocator.get(src);
        WReg amount_reg = register_allocator.get(amount);

        Label label_ge32;
        Label label_else;
        Label label_finish1;
        Label label_finish2;

        assembler.cmp(amount_reg, 0);
        assembler.b(Condition::NE, label_ge32);

        // if amount == 0
        if (opcode.imm) {
            assembler.mov(amount_reg, 32);
            assembler.b(label_ge32);
        } else {
            assembler.mov(result_reg, src_reg);
            assembler.mov(carry_reg, carry_in_reg);
            assembler.b(label_finish1);
        }

        assembler.link(label_ge32);
        assembler.cmp(amount_reg, 31);
        assembler.b(Condition::LE, label_else);

        // if amount >= 32
        assembler.asr(result_reg, src_reg, 31);
        assembler.lsr(carry_reg, src_reg, 31);
        assembler.b(label_finish2);

        // amount > 0 && amount < 32
        assembler.link(label_else);
        assembler.asr(result_reg, src_reg, amount_reg);
        assembler.sub(amount_reg, amount_reg, 1);
        assembler.lsr(carry_reg, src_reg, amount_reg);
        assembler._and(carry_reg, carry_reg, 0x1);

        assembler.link(label_finish1);
        assembler.link(label_finish2);
    } else {
        logger.todo("handle barrel shifter asr case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_barrel_shifter_rotate_right_extended(IRBarrelShifterRotateRightExtended& opcode) {
    const bool src_is_constant = opcode.src.is_constant();
    const bool amount_is_constant = opcode.amount.is_constant();
    WReg result_reg = register_allocator.allocate(opcode.result_and_carry.first);
    WReg carry_reg = register_allocator.allocate(opcode.result_and_carry.second);
    WReg carry_in_reg = register_allocator.get(opcode.carry.as_variable());

    if (opcode.carry.is_constant()) {
        logger.todo("carry in for barrel shifter rrx being constant was not expected");
    }

    if (src_is_constant && amount_is_constant) {
        WReg tmp_msb_reg = register_allocator.allocate_temporary();
        assembler.lsl(tmp_msb_reg, carry_in_reg, 31);

        assembler.mov(carry_reg, opcode.src.as_constant().value & 0x1);

        WReg tmp_value_shifted_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_value_shifted_reg, opcode.src.as_constant().value >> 1);

        assembler.orr(result_reg, tmp_msb_reg, tmp_value_shifted_reg);
    } else if (!src_is_constant && amount_is_constant) {
        const auto src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);

        WReg tmp_msb_reg = register_allocator.allocate_temporary();
        assembler.lsl(tmp_msb_reg, carry_in_reg, 31);

        assembler._and(carry_reg, src_reg, 0x1);

        WReg tmp_value_shifted_reg = register_allocator.allocate_temporary();

        // TODO: combine into 1 instruction
        assembler.lsr(tmp_value_shifted_reg, src_reg, 1);
        assembler.orr(result_reg, tmp_msb_reg, tmp_value_shifted_reg);
    } else {
        logger.todo("handle barrel shifter rrx case %s", opcode.to_string().c_str());
    }
}

void A64Backend::compile_bitwise_and(IRBitwiseAnd& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value & opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_constant();
        WReg lhs_reg = register_allocator.get(lhs);
        assembler._and(dst_reg, lhs_reg, rhs.value);

        if (BitwiseImmediate<32>::is_valid(rhs.value)) {
            assembler._and(dst_reg, lhs_reg, rhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, rhs.value);
            assembler._and(dst_reg, lhs_reg, tmp_imm_reg);
        }
    } else if (!lhs_is_constant && !rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        assembler._and(dst_reg, lhs_reg, rhs_reg);
    } else {
        const auto lhs = opcode.lhs.as_constant();
        const auto rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);

        if (BitwiseImmediate<32>::is_valid(lhs.value)) {
            assembler._and(dst_reg, rhs_reg, lhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, lhs.value);
            assembler._and(dst_reg, rhs_reg, tmp_imm_reg);
        }
    }
}

void A64Backend::compile_bitwise_or(IRBitwiseOr& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value & opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.rhs.as_constant().value);
        assembler.orr(dst_reg, lhs_reg, tmp_imm_reg);
    } else if (!lhs_is_constant && !rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.orr(dst_reg, lhs_reg, rhs_reg);
    } else {
        const auto lhs = opcode.lhs.as_constant();
        const auto rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, lhs.value);
        assembler.orr(dst_reg, tmp_imm_reg, rhs_reg);
    }
}

void A64Backend::compile_bitwise_not(IRBitwiseNot& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);

    if (opcode.src.is_constant()) {
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.src.as_constant().value);
        assembler.mvn(dst_reg, tmp_imm_reg);
    } else {
        auto& src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.mvn(dst_reg, src_reg);
    }
}

void A64Backend::compile_bitwise_exclusive_or(IRBitwiseExclusiveOr& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value ^ opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_constant();
        WReg lhs_reg = register_allocator.get(lhs);

        if (BitwiseImmediate<32>::is_valid(rhs.value)) {
            assembler.eor(dst_reg, lhs_reg, rhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, opcode.rhs.as_constant().value);
            assembler.eor(dst_reg, lhs_reg, tmp_imm_reg);
        }
    } else if (!lhs_is_constant && !rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_variable();
        
        WReg lhs_reg = register_allocator.get(lhs);
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.eor(dst_reg, lhs_reg, rhs_reg);
    } else {
        const auto lhs = opcode.lhs.as_constant();
        const auto rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);

        if (BitwiseImmediate<32>::is_valid(lhs.value)) {
            assembler.eor(dst_reg, rhs_reg, lhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, lhs.value);
            assembler.eor(dst_reg, rhs_reg, tmp_imm_reg);
        }
    }
}

void A64Backend::compile_add(IRAdd& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value + opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_constant();
        WReg lhs_reg = register_allocator.get(lhs);

        if (AddSubImmediate::is_valid(rhs.value)) {
            assembler.add(dst_reg, lhs_reg, rhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, rhs.value);
            assembler.add(dst_reg, lhs_reg, tmp_imm_reg);
        }
    } else if (!lhs_is_constant && !rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_variable();

        WReg lhs_reg = register_allocator.get(lhs);
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.add(dst_reg, lhs_reg, rhs_reg);
    } else {
        const auto lhs = opcode.lhs.as_constant();
        const auto rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);

        if (AddSubImmediate::is_valid(lhs.value)) {
            assembler.add(dst_reg, rhs_reg, lhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, lhs.value);
            assembler.add(dst_reg, tmp_imm_reg, rhs_reg);
        }
    }
}

void A64Backend::compile_add_long(IRAddLong& opcode) {
    WReg dst_upper_reg = register_allocator.allocate(opcode.dst.first);
    WReg dst_lower_reg = register_allocator.allocate(opcode.dst.second);

    WReg lhs_lower_reg;
    WReg lhs_upper_reg;
    WReg rhs_lower_reg;
    WReg rhs_upper_reg;
    XReg lhs_reg = XReg{register_allocator.allocate_temporary().id};
    XReg rhs_reg = XReg{register_allocator.allocate_temporary().id};

    if (opcode.lhs.first.is_constant()) {
        lhs_upper_reg = register_allocator.allocate_temporary();
        assembler.mov(lhs_upper_reg, opcode.lhs.first.as_constant().value);
    } else {
        lhs_upper_reg = register_allocator.get(opcode.lhs.first.as_variable());
    }

    if (opcode.lhs.second.is_constant()) {
        lhs_lower_reg = register_allocator.allocate_temporary();
        assembler.mov(lhs_lower_reg, opcode.lhs.second.as_constant().value);
    } else {
        lhs_lower_reg = register_allocator.get(opcode.lhs.second.as_variable());
    }

    if (opcode.rhs.first.is_constant()) {
        rhs_upper_reg = register_allocator.allocate_temporary();
        assembler.mov(rhs_upper_reg, opcode.rhs.first.as_constant().value);
    } else {
        rhs_upper_reg = register_allocator.get(opcode.rhs.first.as_variable());
    }

    if (opcode.rhs.second.is_constant()) {
        rhs_lower_reg = register_allocator.allocate_temporary();
        assembler.mov(rhs_lower_reg, opcode.rhs.second.as_constant().value);
    } else {
        rhs_lower_reg = register_allocator.get(opcode.rhs.second.as_variable());
    }
    
    assembler.orr(lhs_reg, XReg{lhs_lower_reg.id}, XReg{lhs_upper_reg.id}, Shift::LSL, 32);
    assembler.orr(rhs_reg, XReg{rhs_lower_reg.id}, XReg{rhs_upper_reg.id}, Shift::LSL, 32);

    WReg result_reg = register_allocator.allocate_temporary();
    assembler.add(XReg{result_reg.id}, lhs_reg, rhs_reg);
    assembler.lsr(XReg{dst_upper_reg.id}, XReg{result_reg.id}, 32);
    assembler.mov(dst_lower_reg, result_reg);
}

void A64Backend::compile_subtract(IRSubtract& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value + opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.rhs.as_constant().value);
        assembler.sub(dst_reg, lhs_reg, tmp_imm_reg);
    } else if (!lhs_is_constant && !rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.sub(dst_reg, lhs_reg, rhs_reg);
    } else {
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.lhs.as_constant().value);
        assembler.sub(dst_reg, tmp_imm_reg, rhs_reg);
    }
}

void A64Backend::compile_multiply(IRMultiply& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    
    if (lhs_is_constant && rhs_is_constant) {
        u32 result = opcode.lhs.as_constant().value * opcode.rhs.as_constant().value;
        assembler.mov(dst_reg, result);
    } else if (!lhs_is_constant && rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.rhs.as_constant().value);
        assembler.mul(dst_reg, lhs_reg, tmp_imm_reg);
    } else if (!lhs_is_constant && !rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.mul(dst_reg, lhs_reg, rhs_reg);
    } else {
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, opcode.lhs.as_constant().value);
        assembler.mul(dst_reg, tmp_imm_reg, rhs_reg);
    }
}

void A64Backend::compile_multiply_long(IRMultiplyLong& opcode) {
    WReg dst_upper_reg = register_allocator.allocate(opcode.dst.first);
    WReg dst_lower_reg = register_allocator.allocate(opcode.dst.second);
    WReg lhs_reg;
    WReg rhs_reg;

    if (opcode.lhs.is_constant()) {
        lhs_reg = register_allocator.allocate_temporary();
        assembler.mov(lhs_reg, opcode.lhs.as_constant().value);
    } else {
        lhs_reg = register_allocator.get(opcode.lhs.as_variable());
    }

    if (opcode.rhs.is_constant()) {
        rhs_reg = register_allocator.allocate_temporary();
        assembler.mov(rhs_reg, opcode.rhs.as_constant().value);
    } else {
        rhs_reg = register_allocator.get(opcode.rhs.as_variable());
    }

    if (opcode.is_signed) {
        WReg result_reg = register_allocator.allocate_temporary();
        assembler.smull(XReg{result_reg.id}, lhs_reg, rhs_reg);
        assembler.lsr(XReg{dst_upper_reg.id}, XReg{result_reg.id}, 32);
        assembler.mov(dst_lower_reg, result_reg);
    } else {
        WReg result_reg = register_allocator.allocate_temporary();
        assembler.umull(XReg{result_reg.id}, lhs_reg, rhs_reg);
        assembler.lsr(XReg{dst_upper_reg.id}, XReg{result_reg.id}, 32);
        assembler.mov(dst_lower_reg, result_reg);
    }
}

void A64Backend::compile_compare(IRCompare& opcode) {
    const bool lhs_is_constant = opcode.lhs.is_constant();
    const bool rhs_is_constant = opcode.rhs.is_constant();
    WReg dst_reg = register_allocator.allocate(opcode.dst);

    if (lhs_is_constant && rhs_is_constant) {
        logger.todo("handle when lhs and rhs are both constant");
    } else if (!lhs_is_constant && rhs_is_constant) {
        const auto lhs = opcode.lhs.as_variable();
        const auto rhs = opcode.rhs.as_constant();
        WReg lhs_reg = register_allocator.get(lhs);
        
        if (AddSubImmediate::is_valid(rhs.value)) {
            assembler.cmp(lhs_reg, rhs.value);
        } else {
            WReg tmp_imm_reg = register_allocator.allocate_temporary();
            assembler.mov(tmp_imm_reg, rhs.value);
            assembler.cmp(lhs_reg, tmp_imm_reg);
        }
    } else if (!lhs_is_constant && !rhs_is_constant) {
        auto& lhs = opcode.lhs.as_variable();
        WReg lhs_reg = register_allocator.get(lhs);
        auto& rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        assembler.cmp(lhs_reg, rhs_reg);
    } else {
        const auto lhs = opcode.lhs.as_constant();
        const auto rhs = opcode.rhs.as_variable();
        WReg rhs_reg = register_allocator.get(rhs);
        
        WReg tmp_imm_reg = register_allocator.allocate_temporary();
        assembler.mov(tmp_imm_reg, lhs.value);
        assembler.cmp(tmp_imm_reg, rhs_reg);
    }

    switch (opcode.compare_type) {
    case CompareType::Equal:
        assembler.cset(dst_reg, Condition::EQ);
        break;
    case CompareType::LessThan:
        assembler.cset(dst_reg, Condition::CC);
        break;
    case CompareType::GreaterEqual:
        assembler.cset(dst_reg, Condition::CS);
        break;
    case CompareType::GreaterThan:
        assembler.cset(dst_reg, Condition::HI);
        break;
    }
}

void A64Backend::compile_copy(IRCopy& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    if (opcode.src.is_constant()) {
        const auto src = opcode.src.as_constant();
        assembler.mov(dst_reg, src.value);
    } else {
        const auto src = opcode.src.as_variable();
        WReg src_reg = register_allocator.get(src);
        assembler.mov(dst_reg, src_reg);
    }
}

void A64Backend::compile_get_bit(IRGetBit& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    WReg src_reg;
    
    if (opcode.src.is_constant()) {
        src_reg = register_allocator.allocate_temporary();
        const auto src = opcode.src.as_constant();
        assembler.mov(src_reg, src.value);
    } else {
        src_reg = register_allocator.get(opcode.src.as_variable());
    }

    if (opcode.bit.is_constant()) {
        const auto bit = opcode.bit.as_constant();
        assembler.ubfx(dst_reg, src_reg, bit.value, 1);
    } else {
        const auto bit = opcode.bit.as_variable();
        WReg bit_reg = register_allocator.get(bit);
        assembler.lsr(dst_reg, src_reg, bit_reg);
        assembler._and(dst_reg, dst_reg, 0x1);
    }
}

void A64Backend::compile_set_bit(IRSetBit& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    WReg src_reg;
    WReg value_reg;
    
    if (opcode.src.is_constant()) {
        src_reg = register_allocator.allocate_temporary();
        const auto src = opcode.src.as_constant();
        assembler.mov(src_reg, src.value);
    } else {
        src_reg = register_allocator.get(opcode.src.as_variable());
    }

    if (opcode.value.is_constant()) {
        value_reg = register_allocator.allocate_temporary();
        const auto value = opcode.value.as_constant();
        assembler.mov(value_reg, value.value);
    } else {
        value_reg = register_allocator.get(opcode.value.as_variable());
    }

    if (opcode.bit.is_constant()) {
        const auto bit = opcode.bit.as_constant();
        assembler._and(dst_reg, src_reg, ~(1 << bit.value));
        assembler.orr(dst_reg, dst_reg, value_reg, Shift::LSL, bit.value);
    } else {
        const auto bit = opcode.bit.as_variable();
        const auto bit_reg = register_allocator.get(bit);
        WReg lhs_reg = register_allocator.allocate_temporary();
        WReg rhs_reg = register_allocator.allocate_temporary();
        assembler.mov(lhs_reg, 1);
        assembler.lsl(lhs_reg, lhs_reg, bit_reg);
        assembler._and(lhs_reg, src_reg, lhs_reg);

        assembler.lsl(rhs_reg, value_reg, bit_reg);
        assembler.orr(dst_reg, lhs_reg, rhs_reg);
    }
}

void A64Backend::compile_memory_read(IRMemoryRead& opcode) {
    WReg dst_reg = register_allocator.allocate(opcode.dst);
    WReg addr_reg;
    
    if (opcode.addr.is_constant()) {
        addr_reg = register_allocator.allocate_temporary();
        const auto addr = opcode.addr.as_constant();
        assembler.mov(addr_reg, addr.value);
    } else {
        addr_reg = register_allocator.get(opcode.addr.as_variable());
    }

    // move jit pointer into x0
    assembler.mov(x0, jit_reg);

    // move addr into w1
    assembler.mov(w1, addr_reg);

    // save volatile registers
    push_volatile_registers();

    switch (opcode.access_size) {
    case AccessSize::Byte:
        assembler.invoke_function(reinterpret_cast<void*>(read_byte));
        assembler.mov(dst_reg, w0);
        break;
    case AccessSize::Half:
        switch (opcode.access_type) {
        case AccessType::Aligned:
            assembler.invoke_function(reinterpret_cast<void*>(read_half));
            assembler.mov(dst_reg, w0);
            break;
        case AccessType::Unaligned:
            logger.todo("Jit: handle unaligned half read");
            break;
        }
        
        break;
    case AccessSize::Word:
        if (opcode.access_type == AccessType::Unaligned) {
            assembler.invoke_function(reinterpret_cast<void*>(read_word_rotate));
            assembler.mov(dst_reg, w0);
        } else {
            assembler.invoke_function(reinterpret_cast<void*>(read_word));
            assembler.mov(dst_reg, w0);
        }

        break;
    }

    // restore volatile registers
    pop_volatile_registers();
}

void A64Backend::compile_memory_write(IRMemoryWrite& opcode) {
    WReg addr_reg;
    WReg src_reg;

    if (opcode.addr.is_constant()) {
        addr_reg = register_allocator.allocate_temporary();
        auto& addr = opcode.addr.as_constant();
        assembler.mov(addr_reg, addr.value);
    } else {
        addr_reg = register_allocator.get(opcode.addr.as_variable());
    }

    if (opcode.src.is_constant()) {
        src_reg = register_allocator.allocate_temporary();
        auto& src = opcode.src.as_constant();
        assembler.mov(src_reg, src.value);
    } else {
        src_reg = register_allocator.get(opcode.src.as_variable());
    }

    // move jit pointer into x0
    assembler.mov(x0, jit_reg);

    // move addr and src into w1 and w2 respectively
    assembler.mov(w1, addr_reg);
    assembler.mov(w2, src_reg);

    // save volatile registers
    push_volatile_registers();
    
    switch (opcode.access_size) {
    case AccessSize::Byte:
        assembler.invoke_function(reinterpret_cast<void*>(write_byte));
        break;
    case AccessSize::Half:
        assembler.invoke_function(reinterpret_cast<void*>(write_half));
        break;
    case AccessSize::Word:
        assembler.invoke_function(reinterpret_cast<void*>(write_word));
        break;
    }

    // restore volatile registers
    pop_volatile_registers();
}

} // namespace arm