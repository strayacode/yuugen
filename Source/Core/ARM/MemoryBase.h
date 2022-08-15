#pragma once

#include <array>
#include <string>
#include <fstream>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/Memory.h"
#include "Core/ARM/MMIO.h"

// this is a base class
// which is used by the arm7
// and arm9 memory classes
class MemoryBase {
public:
    MemoryBase(Arch arch) : mmio(arch) {}

    template <typename T>
    T read(u32 addr) {
        addr &= ~(sizeof(T) - 1);

        int index = addr >> PAGE_BITS;
        int offset = addr & PAGE_MASK;

        if (read_page_table[index]) {
            return Common::read<T>(&read_page_table[index][offset]);
        } else {
            if constexpr (sizeof(T) == 1) {
                return slow_read_byte(addr);
            } else if constexpr (sizeof(T) == 2) {
                return slow_read_half(addr);
            } else {
                return slow_read_word(addr);
            }
        }
    }

    template <typename T>
    void write(u32 addr, T data) {
        addr &= ~(sizeof(T) - 1);

        int index = addr >> PAGE_BITS;
        int offset = addr & PAGE_MASK;

        if (write_page_table[index]) {
            Common::write<T>(&write_page_table[index][offset], data);
        } else {
            if constexpr (sizeof(T) == 1) {
                slow_write_byte(addr, data);
            } else if constexpr (sizeof(T) == 2) {
                slow_write_half(addr, data);
            } else {
                slow_write_word(addr, data);
            }
        }
    }

    template <int size>
    std::array<u8, size> load_bios(std::string path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::array<u8, size> bios;

        if (!file) {
            log_fatal("bios could not be found");
        }

        file.unsetf(std::ios::skipws);

        u64 bios_size = file.tellg();

        if (bios_size > size) {
            log_fatal("[MemoryBase] bios has incorrect size! expected: %08x actual: %08x", size, bios_size);
        }

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(bios.data()), size);
        file.close();

        log_debug("bios loaded successfully!");

        return bios;
    }

    static constexpr int PAGE_BITS = 14;
    static constexpr int PAGE_SIZE = 1 << PAGE_BITS;
    static constexpr u32 PAGE_MASK = PAGE_SIZE - 1;

    std::array<u8*, 0x40000> read_page_table = {};
    std::array<u8*, 0x40000> write_page_table = {};
    MMIO mmio;

private:
    virtual u8 slow_read_byte(u32 addr) = 0;
    virtual u16 slow_read_half(u32 addr) = 0;
    virtual u32 slow_read_word(u32 addr) = 0;

    virtual void slow_write_byte(u32 addr, u8 data) = 0;
    virtual void slow_write_half(u32 addr, u16 data) = 0;
    virtual void slow_write_word(u32 addr, u32 data) = 0;
};