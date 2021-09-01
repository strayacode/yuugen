#pragma once

template <bool opcode, bool spsr>
void ARMPSRTransfer() {
    u8 rm = instruction & 0xF;

    if constexpr (opcode) {
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

        if constexpr (spsr) {
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

        if constexpr (spsr) {
            regs.r[rd] = GetCurrentSPSR();
        } else {
            regs.r[rd] = regs.cpsr;
        }
    }
    
    regs.r[15] += 4;
}

template <bool load, bool writeback, bool byte, bool up, bool pre, bool shifted_register>
void ARMSingleDataTransfer() {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    u32 op2 = 0;
    u32 address = regs.r[rn];

    if constexpr (shifted_register) {
        op2 = ARMGetShiftedRegisterSingleDataTransfer();
    } else {
        op2 = instruction & 0xFFF;
    }

    if constexpr (!up) {
        op2 *= -1;
    }

    if constexpr (pre) {
        address += op2;
    }

    // increment r15 by 4 since if we are writing r15 it must be r15 + 12.
    // however if we are loading into r15 this increment won't matter as well
    regs.r[15] += 4;

    if constexpr (load) {
        if constexpr (byte) {
            regs.r[rd] = ReadByte(address);
        } else {
            regs.r[rd] = ReadWordRotate(address);
        }
    } else {
        if constexpr (byte) {
            WriteByte(address, regs.r[rd]);
        } else {
            WriteWord(address, regs.r[rd]);
        }
    }

    if (!load || rd != rn) {
        if constexpr (!pre) {
            regs.r[rn] += op2;
        } else if constexpr (writeback) {
            regs.r[rn] = address;
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

    bool do_writeback = !load || rd != rn;

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

    regs.r[15] += 4;

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
            // cpu locks up when rd is odd
            if (rd & 0x1) {
                log_fatal("undefined ldrd exception");
            }

            regs.r[rd] = ReadWord(address);
            regs.r[rd + 1] = ReadWord(address + 4);

            // when rn == rd + 1 writeback is not performed
            do_writeback = rn != (rd + 1);

            // when rd == 14 the pipeline is flushed
            // due to writing to r15
            if (rd == 14) {
                ARMFlushPipeline();
            }
        }
        break;
    case 0x3:
        if constexpr (load) {
            regs.r[rd] = (s32)(s16)ReadHalf(address);
        } else {
            // cpu locks up when rd is odd
            if (rd & 0x1) {
                log_fatal("undefined strd exception");
            }

            WriteWord(address, regs.r[rd]);
            WriteWord(address + 4, regs.r[rd + 1]);
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    if (do_writeback) {
        if constexpr (!pre) {
            regs.r[rn] += op2;
        } else if constexpr (writeback) {
            regs.r[rn] = address;
        }
    }

    if (rd == 15) {
        log_fatal("handle");
    }
}

template <bool load, bool writeback, bool load_psr, bool up, bool pre>
void ARMBlockDataTransfer() {
    u8 rn = (instruction >> 16) & 0xF;
    u8 r15_in_rlist = (instruction >> 15) & 0x1;
    u32 address = regs.r[rn];
    u8 old_mode = regs.cpsr & 0x1F;

    // make sure to only do user bank transfer if r15 is not in rlist or instruction is not ldm
    // ~(A and B) = ~A or ~B
    bool user_switch_mode = load_psr && (!load || !r15_in_rlist);

    if (user_switch_mode) {
        SwitchMode(USR);
    }

    // u32 old_base = regs.r[rn];

    if constexpr (up) {
        if constexpr (pre) {
            for (int i = 0; i < 16; i++) {
                if (instruction & (1 << i)) {
                    address += 4;
                    if constexpr (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                }
            }
        } else {
            for (int i = 0; i < 16; i++) {
                if (instruction & (1 << i)) {
                    if constexpr (load) {
                        regs.r[i] = ReadWord(address);

                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                    address += 4;
                }
            }
        }
        
    } else {
        if constexpr (pre) {
            for (int i = 15; i >= 0; i--) {
                if (instruction & (1 << i)) {
                    address -= 4;
                    if constexpr (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                }
            }
        } else {
            for (int i = 15; i >= 0; i--) {
                if (instruction & (1 << i)) {
                    if constexpr (load) {
                        regs.r[i] = ReadWord(address);
                    } else {
                        WriteWord(address, regs.r[i]);
                    }
                    address -= 4;
                }
            }
        }
    }

    // TODO: handle writeback edgecases correctly
    if constexpr (writeback) {
        if constexpr (load) {
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
                regs.r[rn] = address;
            }
        }
    } 

    if (user_switch_mode) {
        // switch back to old mode at the end if user bank transfer
        SwitchMode(old_mode);

        if (r15_in_rlist) {
            log_fatal("handle");
            // if (load_psr) {
            //     // cpsr = spsr_<current_mode>
            //     SwitchMode(GetCurrentSPSR());
            // }

            // // since loading spsr_<current_mode> can change the T bit,
            // // we check whether in arm mode or not
            // if (IsARM()) {
            //     ARMFlushPipeline();
            // } else {
            //     ThumbFlushPipeline();
            // }
        }
    }

    if (r15_in_rlist && load) {
        if ((arch == CPUArch::ARMv5) && (regs.r[15] & 0x1)) {
            regs.cpsr |= (1 << 5);
            ThumbFlushPipeline();
        } else {
            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }
}