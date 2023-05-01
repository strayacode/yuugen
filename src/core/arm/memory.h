#pragma once

#include "common/types.h"
#include "common/memory.h"
#include "core/arm/virtual_page_table.h"

namespace core::arm {

enum class Bus {
    // for instructions
    Code,

    // for regular data that gets read by instructions
    Data,

    // for components other than the cpu (e.g. DMA)
    System,

    CodeAndData,
    All,
};

enum RegionAttributes : u8 {
    Read = (1 << 0),
    Write = (1 << 1),
    ReadWrite = Read | Write,
};

// this class will be used as a base class which both the arm7 and arm9 implement
class Memory {
public:
    virtual ~Memory() = default;

    template <typename T, Bus B>
    T read(u32 addr) {
        static_assert(is_one_of_v<T, u8, u16, u32>, "T is not valid");

        auto pointer = get_read_pointer<T, B>(addr);
        if (pointer) {
            return common::read<T>(pointer);
        }

        if (std::is_same_v<T, u8>) {
            return read_byte(addr);
        } else if (std::is_same_v<T, u16>) {
            return read_half(addr);
        } else {
            return read_word(addr);
        }
    }

    template <typename T, Bus B>
    void write(u32 addr, T value) {
        static_assert(is_one_of_v<T, u8, u16, u32>, "T is not valid");

        auto pointer = get_write_pointer<T, B>(addr);
        if (pointer) {
            common::write<T>(pointer, value);
            return;
        }

        if (std::is_same_v<T, u8>) {
            write_byte(addr, value);
        } else if (std::is_same_v<T, u16>) {
            write_half(addr, value);
        } else {
            write_word(addr, value);
        }
    }

    template <Bus B>
    void map(u32 base, u32 end, u8* pointer, u32 mask, RegionAttributes attributes) {
        if (attributes & RegionAttributes::Read) {
            if constexpr (B == Bus::All) {
                get_read_table<Bus::System>().map(base, end, pointer, mask);
                get_read_table<Bus::CodeAndData>().map(base, end, pointer, mask);
            } else {
                get_read_table<B>().map(base, end, pointer, mask);
            }
        }

        if (attributes & RegionAttributes::Write) {
            if constexpr (B == Bus::All) {
                get_write_table<Bus::System>().map(base, end, pointer, mask);
                get_write_table<Bus::CodeAndData>().map(base, end, pointer, mask);
            } else {
                get_write_table<B>().map(base, end, pointer, mask);
            }
        }
    }

    virtual u8 read_byte(u32 addr) = 0;
    virtual u16 read_half(u32 addr) = 0;
    virtual u32 read_word(u32 addr) = 0;

    virtual void write_byte(u32 addr, u8 value) = 0;
    virtual void write_half(u32 addr, u16 value) = 0;
    virtual void write_word(u32 addr, u32 value) = 0;
    
private:
    template <typename T, Bus B>
    u8* get_read_pointer(u32 addr) {
        return get_read_table<B>().template get_pointer<T>(addr);
    }

    template <typename T, Bus B>
    u8* get_write_pointer(u32 addr) {
        return get_write_table<B>().template get_pointer<T>(addr);
    }

    template <Bus B>
    VirtualPageTable<12>& get_read_table() {
        if constexpr (B == Bus::System) {
            return read_system_table;
        } else {
            return read_code_data_table;
        }
    }

    template <Bus B>
    VirtualPageTable<12>& get_write_table() {
        if constexpr (B == Bus::System) {
            return write_system_table;
        } else {
            return write_code_data_table;
        }
    }

    VirtualPageTable<12> read_code_data_table;
    VirtualPageTable<12> write_code_data_table;
    VirtualPageTable<12> read_system_table;
    VirtualPageTable<12> write_system_table;
};

} // namespace core::arm