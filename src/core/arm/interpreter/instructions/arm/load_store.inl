#pragma once

void ARMPSRTransfer() {
    u8 opcode = (instruction >> 21) & 0x1;
    u8 spsr = (instruction >> 22) & 0x1;
    u8 rm = instruction & 0xF;

    if (opcode) {
        // msr
        u8 immediate = (instruction >> 25) & 0x1;
        u32 value = 0;

        u32 mask = 0;
        if (instruction & (1 << 16)) {
            mask |= 0x000000FF;
        }
        if (instruction & (1 << 17)) {
            mask |= 0x0000FF00;
        }
        if (instruction & (1 << 18)) {
            mask |= 0x00FF0000;
        }
        if (instruction & (1 << 19)) {
            mask |= 0xFF000000;
        }

        if (immediate) {
            u32 immediate = instruction & 0xFF;
            u8 rotate_amount = ((instruction >> 8) & 0xF) << 1;

            value = rotate_right(immediate, rotate_amount);
        } else {
            value = regs.r[rm];
        }

        value &= mask;

        if (spsr) {
            if (HasSPSR()) {
                SetCurrentSPSR((GetCurrentSPSR() & ~mask) | value);
            }
        } else {
            if (instruction & (1 << 16) && PrivilegedMode()) {
                SwitchMode(regs.r[rm] & 0x1F);
            }

            regs.cpsr = (regs.cpsr & ~mask) | value;
        }
    } else {
        // mrs
        u8 rd = (instruction >> 12) & 0xF;

        if (spsr) {
            regs.r[rd] = GetCurrentSPSR();
        } else {
            regs.r[rd] = regs.cpsr;
        }
    }
    
    regs.r[15] += 4;
}

void ARMSingleDataTransfer() {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u8 load = (instruction >> 20) & 0x1;
    u8 writeback = (instruction >> 21) & 0x1;
    u8 byte = (instruction >> 22) & 0x1;
    u8 up = (instruction >> 23) & 0x1;
    u8 pre = (instruction >> 24) & 0x1;
    u8 shifted_register = (instruction >> 25) & 0x1;

    u32 op2 = 0;
    u32 address = regs.r[rn];

    if (shifted_register) {
        op2 = ARMGetShiftedRegisterSingleDataTransfer();
    } else {
        op2 = instruction & 0xFFF;
    }

    if (!up) {
        op2 *= -1;
    }

    if (pre) {
        address += op2;
    }

    // increment r15 by 4 since if we are writing r15 it must be r15 + 12.
    // however if we are loading into r15 this increment won't matter as well
    regs.r[15] += 4;

    if (load) {
        if (byte) {
            regs.r[rd] = ReadByte(address);
        } else {
            regs.r[rd] = ReadWord(address);

            if (address & 0x3) {
                u8 shift_amount = (address & 0x3) * 8;
                regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
            }
        }
    } else {
        if (byte) {
            WriteByte(address, regs.r[rd]);
        } else {
            WriteWord(address, regs.r[rd]);
        }
    }

    // for ldr instructons writeback can't happen when rd != rn
    if (!load || rd != rn) {
        if (writeback || !pre) {
            regs.r[rn] += op2;
        }
    }

    if (load && rd == 15) {
        if ((arch == CPUArch::ARMv5) && (regs.r[15] & 1)) {
            regs.cpsr |= 1 << 5;
            regs.r[15] &= ~1;
            ThumbFlushPipeline();
        } else {
            regs.r[15] &= ~3;
            ARMFlushPipeline();
        }
    }
}

auto ARMGetShiftedRegisterSingleDataTransfer() -> u32 {
    u8 shift_type = (instruction >> 5) & 0x3;
    u8 shift_amount = (instruction >> 7) & 0x1F;

    u8 rm = instruction & 0xF;

    u32 op2 = 0;

    switch (shift_type) {
    case 0x0:
        // LSL
        op2 = regs.r[rm] << shift_amount;
        break;
    case 0x1:
        // LSR
        if (shift_amount != 0) {
            op2 = regs.r[rm] >> shift_amount;
        }
        break;
    case 0x2: {
        // ASR
        u8 msb = regs.r[rm] >> 31;

        if (shift_amount == 0) {
            op2 = 0xFFFFFFFF * msb;
        } else {
            op2 = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        // ROR
        if (shift_amount == 0) {
            // rotate right extended
            op2 = (GetConditionFlag(C_FLAG) << 31) | (regs.r[rm] >> 1);
        } else {
            // rotate right
            op2 = rotate_right(regs.r[rm], shift_amount);
        }
        break;
    }

    return op2;
}

template <bool load, bool writeback, bool immediate, bool up, bool pre>
void ARMHalfwordDataTransfer() {
    u8 rm = instruction & 0xF;
    u8 opcode = (instruction >> 5) & 0x3;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 op2 = 0;
    u32 address = regs.r[rn];

    if constexpr (immediate) {
        op2 = ((instruction >> 4) & 0xF0) | (instruction & 0xF);
    } else {
        op2 = regs.r[rm];
    }

    if constexpr (!up) {
        op2 *= -1;
    }

    if constexpr (pre) {
        address += op2;
    }

    switch (opcode) {
    case 0x1:
        if constexpr (load) {
            regs.r[rd] = ReadHalf(address);
        } else {
            WriteHalf(address, regs.r[rd]);
        }
        break;
    case 0x2:
        if constexpr (load) {
            regs.r[rd] = (s32)(s8)ReadByte(address);
        } else {
            log_fatal("handle ldrd");
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    // for ldrh instructons writeback can't happen when rd != rn
    if (!load || rd != rn) {
        if constexpr (writeback || !pre) {
            regs.r[rn] += op2;
        }
    }

    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

// TODO: optimise this
void ARMBlockDataTransfer() {
    u8 rn = (instruction >> 16) & 0xF;
    u8 load = (instruction >> 20) & 0x1;
    u8 writeback = (instruction >> 21) & 0x1;
    u8 load_psr = (instruction >> 22) & 0x1;
    u8 up = (instruction >> 23) & 0x1;
    u8 pre = (instruction >> 24) & 0x1;

    u8 r15_in_rlist = (instruction >> 15) & 0x1;
    
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    // make sure to only do user bank transfer if r15 is not in rlist or instruction is not ldm
    // ~(A and B) = ~A or ~B
    bool user_switch_mode = load_psr && (!load || !r15_in_rlist);

    if (user_switch_mode) {
        SwitchMode(USR);
    }

    u32 old_base = regs.r[rn];

    if (up) {
        if (pre) {
            for (int i = 0; i < 16; i++) {
                if (instruction & (1 << i)) {
                    address += 4;
                    if (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                }
            }
        } else {
            for (int i = 0; i < 16; i++) {
                if (instruction & (1 << i)) {
                    if (load) {
                        regs.r[i] = ReadWord(address);

                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                    address += 4;
                }
            }
        }
        
    } else {
        if (pre) {
            for (int i = 15; i >= 0; i--) {
                if (instruction & (1 << i)) {
                    address -= 4;
                    if (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                }
            }
        } else {
            for (int i = 15; i >= 0; i--) {
                if (instruction & (1 << i)) {
                    if (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                    address -= 4;
                }
            }
        }
    }

    if (writeback) {
        if (load) {
            if (arch == CPUArch::ARMv5) {
                if (((instruction & 0xFFFF) == (unsigned int)(1 << rn)) || !(((instruction & 0xFFFF) >> rn) == 1)) {
                    regs.r[rn] = address;
                }
            } else {
                if (!(instruction & (unsigned int)(1 << rn))) {
                    regs.r[rn] = address;
                }
            }
        } else {
            if (arch == CPUArch::ARMv5) {
                regs.r[rn] = address;
            } else {
                if (instruction & (1 << rn)) {
                    bool rn_first = true;

                    for (int i = 0; i < rn; i++) {
                        if (instruction & (1 << i)) {
                            rn_first = false;
                        }
                    }

                    if (rn_first) {
                        regs.r[rn] = old_base;
                    } else {
                        regs.r[rn] = address;
                    }
                } else {
                    regs.r[rn] = address;
                }
            }
        }
    } 

    if (user_switch_mode) {
        // switch back to old mode at the end if user bank transfer
        SwitchMode(old_mode);
    }

    if (r15_in_rlist && load) {
        if (load_psr) {
            // cpsr = spsr_<current_mode>
            SwitchMode(GetCurrentSPSR());
        }

        // since loading spsr_<current_mode> can change the T bit,
        // we check whether in arm mode or not
        if (IsARM()) {
            ARMFlushPipeline();
        } else {
            ThumbFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
}