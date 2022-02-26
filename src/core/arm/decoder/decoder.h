#pragma once

#include <array>
#include <string>
#include <algorithm>
#include <vector>
#include <assert.h>
#include "common/types.h"
#include "common/arithmetic.h"

// general idea:
// . have an arm lut and thumb where each item contains function pointer to the function we want to call
// . use a template which corresponds to a class of the functions we want to call
// . there are 2 cases where in one case the class that is provided as the template parameter (e.g. ARMDisassembler) will be calling the returned
// function pointer, in which case the method doesn't need to be static. but otherwise if we are planning to call a class's member function pointer from elsewhere
// (e.g. jit doing interpreter fallback) then the method must be static for it to work like a regular function pointer
template <typename D, typename Callback = decltype(&D::unknown_instruction)>
class Decoder {
public:
    struct InstructionInfo {
        Callback callback;
        u32 mask;
        u32 value;
    };

    Decoder() {
        arm_lut.fill(&D::unknown_instruction);
        thumb_lut.fill(&D::unknown_instruction);

        register_arm("101xxxxxxxxx", &D::ARMBranchLinkMaybeExchange);
        register_arm("000100100001", &D::ARMBranchExchange);
        register_arm("000101100001", &D::ARMCountLeadingZeroes);
        register_arm("000100100011", &D::ARMBranchLinkExchangeRegister);
        register_arm("00010x001001", &D::ARMSingleDataSwap);
        register_arm("000000xx1001", &D::ARMMultiply);
        register_arm("00010xx00101", &D::ARMSaturatingAddSubtract);
        register_arm("00001xxx1001", &D::ARMMultiplyLong);
        register_arm("00x10xx0xxxx", &D::ARMPSRTransfer);
        register_arm("000xxxxx1xx1", &D::ARMHalfwordDataTransfer);
        register_arm("100xxxxxxxxx", &D::ARMBlockDataTransfer);
        register_arm("01xxxxxxxxxx", &D::ARMSingleDataTransfer);
        register_arm("00xxxxxxxxxx", &D::ARMDataProcessing);
        register_arm("1110xxxxxxx1", &D::ARMCoprocessorRegisterTransfer);
        register_arm("1111xxxxxxxx", &D::ARMSoftwareInterrupt);
        register_arm("000101001xx0", &D::ARMSignedHalfwordAccumulateLong);
        register_arm("000100101xx0", &D::ARMSignedHalfwordWordMultiply);
        register_arm("00010xx01xx0", &D::ARMSignedHalfwordMultiply);
        register_arm("000100100111", &D::ARMBreakpoint);
        
        std::stable_sort(arm_list.begin(), arm_list.end(), [](InstructionInfo a, InstructionInfo b) {
            return bit_count(a.mask) > bit_count(b.mask);
        });

        for (u64 i = 0; i < arm_lut.size(); i++) {
            for (InstructionInfo info : arm_list) {
                if ((i & info.mask) == info.value) {
                    arm_lut[i] = info.callback;
                    break;
                }
            }
        }

        register_thumb("001xxxxxxx", &D::ThumbALUImmediate);
        register_thumb("11111xxxxx", &D::ThumbBranchLinkOffset);
        register_thumb("11110xxxxx", &D::ThumbBranchLinkSetup);
        register_thumb("11101xxxxx", &D::ThumbBranchLinkExchangeOffset);
        register_thumb("11100xxxxx", &D::ThumbBranch);
        register_thumb("1011x10xxx", &D::ThumbPushPop);
        register_thumb("010000xxxx", &D::ThumbDataProcessingRegister);
        register_thumb("010001xxxx", &D::ThumbSpecialDataProcesing);
        register_thumb("010001111x", &D::ThumbBranchLinkExchange);
        register_thumb("010001110x", &D::ThumbBranchExchange);
        register_thumb("0101xxxxxx", &D::ThumbLoadStore);
        register_thumb("01001xxxxx", &D::ThumbLoadPC);
        register_thumb("1001xxxxxx", &D::ThumbLoadStoreSPRelative);
        register_thumb("1000xxxxxx", &D::ThumbLoadStoreHalfword);
        register_thumb("00011xxxxx", &D::ThumbAddSubtract);
        register_thumb("000xxxxxxx", &D::ThumbShiftImmediate);
        register_thumb("11011111xx", &D::ThumbSoftwareInterrupt);
        register_thumb("1101xxxxxx", &D::ThumbBranchConditional);
        register_thumb("1100xxxxxx", &D::ThumbLoadStoreMultiple);
        register_thumb("011xxxxxxx", &D::ThumbLoadStoreImmediate);
        register_thumb("1010xxxxxx", &D::ThumbAddSPPC);
        register_thumb("10110000xx", &D::ThumbAdjustStackPointer);
        
        std::stable_sort(thumb_list.begin(), thumb_list.end(), [](InstructionInfo a, InstructionInfo b) {
            return bit_count(a.mask) > bit_count(b.mask);
        });

        for (u64 i = 0; i < thumb_lut.size(); i++) {
            for (InstructionInfo info : thumb_list) {
                if ((i & info.mask) == info.value) {
                    thumb_lut[i] = info.callback;
                    break;
                }
            }
        }
    }

    Callback decode_arm(u32 instruction) {
        u32 index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
        return arm_lut[index];
    }

    Callback decode_thumb(u16 instruction) {
        u32 index = instruction >> 6;
        return thumb_lut[index];
    }

    void register_arm(std::string pattern, Callback callback) {
        u32 mask = create_pattern_mask<u32>(pattern);
        u32 value = create_pattern_value<u32>(pattern);

        arm_list.push_back({callback, mask, value});
    }

    void register_thumb(std::string pattern, Callback callback) {
        u16 mask = create_pattern_mask<u16>(pattern);
        u16 value = create_pattern_value<u16>(pattern);

        thumb_list.push_back({callback, mask, value});
    }

    template <typename T>
    T create_pattern_mask(std::string pattern) {
        T result = 0;
        int n = num_bits<T>();
        int i = 0;

        while (pattern[i] != '\0') {
            if (pattern[i] == '0' || pattern[i] == '1') {
                result |= (1 << (n - i - 1));
            }

            i++;
        }

        result >>= n - i;

        return result;
    }

    template <typename T>
    T create_pattern_value(std::string pattern) {
        T result = 0;
        int n = num_bits<T>();
        int i = 0;

        while (pattern[i] != '\0') {
            if (pattern[i] == '1') {
                result |= (1 << (n - i - 1));
            }

            i++;
        }

        result >>= n - i;

        return result;
    }

    template <typename T>
    int num_bits() {
        return sizeof(T) * 8;
    }

private:
    std::vector<InstructionInfo> arm_list;
    std::vector<InstructionInfo> thumb_list;
    std::array<Callback, 1024> thumb_lut;
    std::array<Callback, 4096> arm_lut;
};