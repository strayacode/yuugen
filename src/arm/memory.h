#pragma once

#include "common/types.h"
#include "common/logger.h"
#include "common/memory.h"
#include "arm/coprocessor.h"
#include "arm/page_table.h"

namespace arm {

enum class Bus {
    // for instructions
    Code,

    // for regular data that gets read by instructions
    Data,

    // for components other than the cpu (e.g. DMA)
    System,
};

enum RegionAttributes : u8 {
    Read = (1 << 0),
    Write = (1 << 1),
    ReadWrite = Read | Write,
};

class Memory {
public:
    virtual ~Memory() = default;

    template <typename T, Bus B>
    T read(u32 addr) {
        static_assert(is_one_of_v<T, u8, u16, u32>, "T is not valid");
        addr &= ~(sizeof(T) - 1);

        // TODO: add back
        // if constexpr (B != Bus::System) {
            if (itcm.config.enable_reads && addr >= itcm.config.base && addr < itcm.config.limit) {
                return common::read<T>(itcm.data, (addr - itcm.config.base) & itcm.mask);
            }
        // }

        // TODO: add back
        // if constexpr (B == Bus::Data) {
            if (dtcm.config.enable_reads && addr >= dtcm.config.base && addr < dtcm.config.limit) {
                return common::read<T>(dtcm.data, (addr - dtcm.config.base) & dtcm.mask);
            }
        // }

        auto pointer = read_table.get_pointer<T>(addr);
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
        addr &= ~(sizeof(T) - 1);

        // TODO: add back
        // if constexpr (B != Bus::System) {
            if (itcm.config.enable_writes && addr >= itcm.config.base && addr < itcm.config.limit) {
                common::write<T>(itcm.data, value, (addr - itcm.config.base) & itcm.mask);
                return;
            }
        // }

        // TODO: add back
        // if constexpr (B == Bus::Data) {
            if (dtcm.config.enable_writes && addr >= dtcm.config.base && addr < dtcm.config.limit) {
                common::write<T>(dtcm.data, value, (addr - dtcm.config.base) & dtcm.mask);
                return;
            }
        // }

        auto pointer = write_table.get_pointer<T>(addr);
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

    void map(u32 base, u32 end, u8* pointer, u32 mask, RegionAttributes attributes) {
        if (attributes & RegionAttributes::Read) {
            read_table.map(base, end, pointer, mask);
        }

        if (attributes & RegionAttributes::Write) {
            write_table.map(base, end, pointer, mask);
        }
    }

    void unmap(u32 base, u32 end, RegionAttributes attributes) {
        if (attributes & RegionAttributes::Read) {
            read_table.unmap(base, end);
        }

        if (attributes & RegionAttributes::Write) {
            write_table.unmap(base, end);
        }
    }

    virtual u8 read_byte(u32 addr) = 0;
    virtual u16 read_half(u32 addr) = 0;
    virtual u32 read_word(u32 addr) = 0;

    virtual void write_byte(u32 addr, u8 value) = 0;
    virtual void write_half(u32 addr, u16 value) = 0;
    virtual void write_word(u32 addr, u32 value) = 0;

    Coprocessor::TCM dtcm;
    Coprocessor::TCM itcm;
    
private:
    PageTable<14> read_table;
    PageTable<14> write_table;
};

} // namespace arm