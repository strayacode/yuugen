#include "common/lut_helpers.h"
#include "core/arm/cpu_core.h"

template <u32 instruction>
static constexpr Instruction GetARMInstruction() {
    switch ((instruction >> 25) & 0x7) {
    case 0x0: {
        const bool set_flags = instruction & (1 << 20);
        const u8 opcode = (instruction >> 21) & 0xF;
        const u8 shift_imm = (instruction >> 25) & 0x1;

        if ((instruction & 0x90) == 0x90) {
            // multiplies, extra load/stores
            if ((instruction & 0x60) == 0) {
                const bool set_flags = (instruction >> 20) & 0x1;
                const bool accumulate = (instruction >> 21) & 0x1;
                switch ((instruction >> 23) & 0x3) {
                case 0x0: {
                    return &CPUCore::ARMMultiply<accumulate, set_flags>;
                }
                case 0x1: {
                    const bool sign = instruction & (1 << 22);
                    return &CPUCore::ARMMultiplyLong<accumulate, set_flags, sign>;
                }
                case 0x2:
                    return &CPUCore::ARMSingleDataSwap;
                }
            }

            const bool load = (instruction >> 20) & 0x1;
            const bool writeback = (instruction >> 21) & 0x1;
            const bool immediate = (instruction >> 22) & 0x1;
            const bool up = (instruction >> 23) & 0x1;
            const bool pre = (instruction >> 24) & 0x1;

            return &CPUCore::ARMHalfwordDataTransfer<load, writeback, immediate, up, pre>;
        } else if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            // miscellaneous instructions
            if ((instruction & 0xF0) == 0) {
                return &CPUCore::ARMPSRTransfer<(instruction >> 21) & 0x1, (instruction >> 22) & 0x1>;
            }
            
            if ((instruction & 0xFF000F0) == 0x1200010) {
                return &CPUCore::ARMBranchExchange;
            }
            
            if ((instruction & 0xFF000F0) == 0x1600010) {
                return &CPUCore::ARMCountLeadingZeroes;
            }
            
            if ((instruction & 0xFF000F0) == 0x1200030) {
                return &CPUCore::ARMBranchLinkExchangeRegister;
            }
            
            if ((instruction & 0xF0) == 0x50) {
                return &CPUCore::ARMSaturatingAddSubtract;
            }
            
            if ((instruction & 0x70) == 0x70) {
                return &CPUCore::ARMBreakpoint;
            }
            
            if ((instruction & 0x90) == 0x80) {
                switch ((instruction >> 21) & 0xF) {
                case 0x8:
                    return &CPUCore::ARMSignedHalfwordMultiply<true>;
                case 0x9:
                    return &CPUCore::ARMSignedHalfwordWordMultiply;
                case 0xA:
                    return &CPUCore::ARMSignedHalfwordAccumulateLong;
                case 0xB:
                    return &CPUCore::ARMSignedHalfwordMultiply<false>;
                default:
                    return &CPUCore::ARMSignedHalfwordMultiply<false>;
                }
            }
        }

        return &CPUCore::ARMDataProcessing<shift_imm, set_flags>;
    }
    case 0x1: {
        const bool set_flags = instruction & (1 << 20);
        const u8 opcode = (instruction >> 21) & 0xF;
        const bool shift_imm = (instruction >> 25) & 0x1;

        if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            if (instruction & (1 << 21)) {
                return &CPUCore::ARMPSRTransfer<(instruction >> 21) & 0x1, (instruction >> 22) & 0x1>;
            }
            
            return &CPUCore::ARMUndefined;
        }

        return &CPUCore::ARMDataProcessing<shift_imm, set_flags>;
    }
    case 0x2: case 0x3: {
        const bool load = (instruction >> 20) & 0x1;
        const bool writeback = (instruction >> 21) & 0x1;
        const bool byte = (instruction >> 22) & 0x1;
        const bool up = (instruction >> 23) & 0x1;
        const bool pre = (instruction >> 24) & 0x1;
        const bool shifted_register = (instruction >> 25) & 0x1;

        return &CPUCore::ARMSingleDataTransfer<load, writeback, byte, up, pre, shifted_register>;
    }
    case 0x4: {
        const bool load = (instruction >> 20) & 0x1;
        const bool writeback = (instruction >> 21) & 0x1;
        const bool load_psr = (instruction >> 22) & 0x1;
        const bool up = (instruction >> 23) & 0x1;
        const bool pre = (instruction >> 24) & 0x1;

        return &CPUCore::ARMBlockDataTransfer<load, writeback, load_psr, up, pre>;
    }
    case 0x5: {
        // b/bl/blx
        const bool link = instruction & (1 << 24);

        return &CPUCore::ARMBranchLinkMaybeExchange<link>;
    }
    case 0x7:
        if ((instruction & 0x1000010) == 0x10) {
            return &CPUCore::ARMCoprocessorRegisterTransfer;
        } else if ((instruction & 0x1000010) == 0) {
            return &CPUCore::UnimplementedInstruction;
        }

        return &CPUCore::ARMSoftwareInterrupt;
    default:
        return &CPUCore::UnimplementedInstruction;
    }
}

template <u16 instruction>
static constexpr Instruction GetThumbInstruction() {
    switch ((instruction >> 13) & 0x7) {
    case 0x0:
        if (((instruction >> 11) & 0x3) == 0x3) {
            return &CPUCore::ThumbAddSubtract;
        }

        return &CPUCore::ThumbShiftImmediate;
    case 0x1:
        return &CPUCore::ThumbALUImmediate;
    case 0x2:
        if (((instruction >> 10) & 0x7) == 0x0) {
            return &CPUCore::ThumbDataProcessingRegister;
        } else if (((instruction >> 10) & 0x7) == 0x1) {
            if ((instruction & 0xFF00) == 0x4700) {
                if (instruction & (1 << 7)) {
                    return &CPUCore::ThumbBranchLinkExchange;
                }

                return &CPUCore::ThumbBranchExchange;
            }

            return &CPUCore::ThumbSpecialDataProcesing;
        }

        if (((instruction >> 12) & 0x1) == 0x1) {
            return &CPUCore::ThumbLoadStore;
        }
        
        return &CPUCore::ThumbLoadPC;
    case 0x3:
        return &CPUCore::ThumbLoadStoreImmediate;
    case 0x4:
        if (instruction & (1 << 12)) {
            return &CPUCore::ThumbLoadStoreSPRelative;
        }

        return &CPUCore::ThumbLoadStoreHalfword;
    case 0x5:
        if (instruction & (1 << 12)) {
            // miscellaneous
            if (((instruction >> 8) & 0x1F) == 0x10) {
                return &CPUCore::ThumbAdjustStackPointer;
            } else if (((instruction >> 8) & 0x1F) == 0x1E) {
                return &CPUCore::ThumbSoftwareInterrupt;
            }

            return &CPUCore::ThumbPushPop;
        }

        return &CPUCore::ThumbAddSPPC;
    case 0x6:
        if (!(instruction & (1 << 12))) {
            return &CPUCore::ThumbLoadStoreMultiple;
        } else if ((instruction & 0xFF00) == 0xDF00) {
            return &CPUCore::ThumbSoftwareInterrupt;
        }

        return &CPUCore::ThumbBranchConditional;
    case 0x7:
        if (instruction & (1 << 12)) {
            if (instruction & (1 << 11)) {
                return &CPUCore::ThumbBranchLinkOffset;
            }

            return &CPUCore::ThumbBranchLinkSetup;
        } else {
            if (instruction & (1 << 11)) {
                return &CPUCore::ThumbBranchLinkExchangeOffset;
            }

            return &CPUCore::ThumbBranch;
        }
    default:
        return &CPUCore::UnimplementedInstruction;
    }
}

static constexpr auto StaticGenerateARMTable() -> std::array<Instruction, 4096> {
    std::array<Instruction, 4096> arm_lut = {};

    static_for<std::size_t, 0, 4096>([&](auto i) {
        arm_lut[i] = GetARMInstruction<((i & 0xFF0) << 16) | ((i & 0xF) << 4)>();
    }); 

    return arm_lut;
}

static constexpr auto StaticGenerateThumbTable() -> std::array<Instruction, 1024> {
    std::array<Instruction, 1024> thumb_lut = {};

    static_for<std::size_t, 0, 1024>([&](auto i) {
        thumb_lut[i] = GetThumbInstruction<i << 6>();
    }); 

    return thumb_lut;
}

void CPUCore::GenerateARMTable() {
    arm_lut = StaticGenerateARMTable();
}

void CPUCore::GenerateThumbTable() {
    thumb_lut = StaticGenerateThumbTable();
}