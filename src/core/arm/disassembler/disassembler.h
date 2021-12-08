#pragma once

#include <string>
#include "common/types.h"
#include "core/arm/disassembler/arm/alu.h"
#include "core/arm/disassembler/arm/branch.h"
#include "core/arm/disassembler/arm/load_store.h"

std::string DisassembleARMInstruction(u32 instruction, u32 pc) {
    const bool set_flags = instruction & (1 << 20);
    const u8 opcode = (instruction >> 21) & 0xF;

    switch ((instruction >> 25) & 0x7) {
    case 0x0:
        if ((instruction & 0x90) == 0x90) {
            // multiplies, extra load/stores
            if ((instruction & 0x60) == 0) {
                switch ((instruction >> 23) & 0x3) {
                case 0x0:
                    return DisassembleARMMultiply(instruction);
                case 0x1:
                    // return &CPUCore::ARMMultiplyLong<accumulate, set_flags, sign>;
                    log_fatal("handle");
                case 0x2:
                    // return &CPUCore::ARMSingleDataSwap;
                    log_fatal("handle");
                }
            }

            return DisassembleARMHalfwordDataTransfer(instruction);
        } else if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            // miscellaneous instructions
            if ((instruction & 0xF0) == 0) {
                return DisassembleARMStatusTransfer(instruction);
            }
            
            if ((instruction & 0xFF000F0) == 0x1200010) {
                return DisassembleARMBranchExchange(instruction);
            }
            
            if ((instruction & 0xFF000F0) == 0x1600010) {
                return DisassembleARMCountLeadingZeroes(instruction);
            }
            
            if ((instruction & 0xFF000F0) == 0x1200030) {
                return DisassembleARMBranchLinkExchangeRegister(instruction);
            }
            
            if ((instruction & 0xF0) == 0x50) {
                // return &CPUCore::ARMSaturatingAddSubtract;
                log_fatal("handle");
            }
            
            if ((instruction & 0x70) == 0x70) {
                // return &CPUCore::ARMBreakpoint;
                log_fatal("handle");
            }
            
            if ((instruction & 0x90) == 0x80) {
                switch ((instruction >> 21) & 0xF) {
                case 0x8:
                    // return &CPUCore::ARMSignedHalfwordMultiply<true>;
                    log_fatal("handle");
                case 0x9:
                    // return &CPUCore::ARMSignedHalfwordWordMultiply;
                    log_fatal("handle");
                case 0xA:
                    // return &CPUCore::ARMSignedHalfwordAccumulateLong;
                    log_fatal("handle");
                case 0xB:
                    // return &CPUCore::ARMSignedHalfwordMultiply<false>;
                    log_fatal("handle");
                default:
                    // return &CPUCore::ARMSignedHalfwordMultiply<false>;
                    log_fatal("handle");
                }
            }
        }

        return DisassembleARMDataProcessing(instruction);
    case 0x1:
        if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            if (instruction & (1 << 21)) {
                return DisassembleARMStatusTransfer(instruction);
            }
            
            return "undefined";
        }

        return DisassembleARMDataProcessing(instruction);
    case 0x2: case 0x3:
        return DisassembleARMSingleDataTransfer(instruction);
    case 0x4:
        return DisassembleARMBlockDataTransfer(instruction);
    case 0x5:
        return DisassembleARMBranchLinkMaybeExchange(instruction, pc);
    case 0x7:
        if ((instruction & 0x1000010) == 0x10) {
            return DisassembleARMCoprocessorRegisterTransfer(instruction);
        } else if ((instruction & 0x1000010) == 0) {
            // return &CPUCore::UnimplementedInstruction;
            log_fatal("handle");
        }

        return DisassembleARMSoftwareInterrupt(instruction);
    default:
        return "";
    }
}

std::string DisassembleThumbInstruction(u16 instruction, u32 pc) {
    return "n/a";
}

