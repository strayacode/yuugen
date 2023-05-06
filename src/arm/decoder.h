#pragma once

#include <array>
#include <string>
#include <algorithm>
#include <vector>
#include "common/types.h"
#include "common/bits.h"

namespace arm {

template <typename D, typename Callback = decltype(&D::illegal_instruction)>
class Decoder {
public:
    struct InstructionInfo {
        Callback callback;
        u32 mask;
        u32 value;
    };

    Decoder() {
        arm_lut.fill(&D::illegal_instruction);
        thumb_lut.fill(&D::illegal_instruction);

        register_arm("101xxxxxxxxx", &D::arm_branch_link_maybe_exchange);
        register_arm("000100100001", &D::arm_branch_exchange);
        register_arm("000101100001", &D::arm_count_leading_zeroes);
        register_arm("000100100011", &D::arm_branch_link_exchange_register);
        register_arm("00010x001001", &D::arm_single_data_swap);
        register_arm("000000xx1001", &D::arm_multiply);
        register_arm("00010xx00101", &D::arm_saturating_add_subtract);
        register_arm("00001xxx1001", &D::arm_multiply_long);
        register_arm("000xxxxx1xx1", &D::arm_halfword_data_transfer);
        register_arm("00x10x00xxxx", &D::arm_status_load);
        register_arm("00x10x10xxxx", &D::arm_status_store);
        register_arm("100xxxxxxxxx", &D::arm_block_data_transfer);
        register_arm("01xxxxxxxxxx", &D::arm_single_data_transfer);
        register_arm("00xxxxxxxxxx", &D::arm_data_processing);
        register_arm("1110xxxxxxx1", &D::arm_coprocessor_register_transfer);
        register_arm("1111xxxxxxxx", &D::arm_software_interrupt);
        register_arm("000101001xx0", &D::arm_signed_multiply_accumulate_long);
        register_arm("000100101xx0", &D::arm_signed_multiply_word);
        register_arm("00010xx01xx0", &D::arm_signed_multiply);
        register_arm("000100100111", &D::arm_breakpoint);
        
        std::stable_sort(arm_list.begin(), arm_list.end(), [](InstructionInfo a, InstructionInfo b) {
            return common::bit_count(a.mask) > common::bit_count(b.mask);
        });

        for (u64 i = 0; i < arm_lut.size(); i++) {
            for (InstructionInfo info : arm_list) {
                if ((i & info.mask) == info.value) {
                    arm_lut[i] = info.callback;
                    break;
                }
            }
        }

        register_thumb("001xxxxxxx", &D::thumb_alu_immediate);
        register_thumb("11111xxxxx", &D::thumb_branch_link_offset);
        register_thumb("11110xxxxx", &D::thumb_branch_link_setup);
        register_thumb("11101xxxxx", &D::thumb_branch_link_exchange_offset);
        register_thumb("11100xxxxx", &D::thumb_branch);
        register_thumb("1011x10xxx", &D::thumb_push_pop);
        register_thumb("010000xxxx", &D::thumb_data_processing_register);
        register_thumb("010001xxxx", &D::thumb_special_data_processing);
        register_thumb("010001111x", &D::thumb_branch_link_exchange);
        register_thumb("010001110x", &D::thumb_branch_exchange);
        register_thumb("0101xxxxxx", &D::thumb_load_store);
        register_thumb("01001xxxxx", &D::thumb_load_pc);
        register_thumb("1001xxxxxx", &D::thumb_load_store_sp_relative);
        register_thumb("1000xxxxxx", &D::thumb_load_store_halfword);
        register_thumb("00011xxxxx", &D::thumb_add_subtract);
        register_thumb("000xxxxxxx", &D::thumb_shift_immediate);
        register_thumb("11011111xx", &D::thumb_software_interrupt);
        register_thumb("1101xxxxxx", &D::thumb_branch_conditional);
        register_thumb("1100xxxxxx", &D::thumb_load_store_multiple);
        register_thumb("011xxxxxxx", &D::thumb_load_store_immediate);
        register_thumb("1010xxxxxx", &D::thumb_add_sp_pc);
        register_thumb("10110000xx", &D::thumb_adjust_stack_pointer);
        
        std::stable_sort(thumb_list.begin(), thumb_list.end(), [](InstructionInfo a, InstructionInfo b) {
            return common::bit_count(a.mask) > common::bit_count(b.mask);
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

    Callback get_arm_handler(u32 instruction) {
        u32 index = ((instruction >> 16) & 0xff0) | ((instruction >> 4) & 0xf);
        return arm_lut[index];
    }

    Callback get_thumb_handler(u16 instruction) {
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
    T create_pattern_mask(const std::string& pattern) {
        T result = 0;
        int n = common::num_bits<T>();
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
    T create_pattern_value(const std::string& pattern) {
        T result = 0;
        int n = common::num_bits<T>();
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

private:
    std::vector<InstructionInfo> arm_list;
    std::vector<InstructionInfo> thumb_list;
    std::array<Callback, 1024> thumb_lut;
    std::array<Callback, 4096> arm_lut;
};

} // namespace arm