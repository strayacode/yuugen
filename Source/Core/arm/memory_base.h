#pragma once

#include <array>
#include <string>
#include <fstream>
#include <string.h>
#include <assert.h>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/Callback.h"

// this is a base class
// which is used by the arm7
// and arm9 memory classes
class MemoryBase {
public:
    template <typename T>
    T FastRead(u32 addr) {
        addr &= ~(sizeof(T) - 1);

        T return_value = 0;

        int index = addr >> 12;
        int offset = addr & 0xFFF;

        if (read_page_table[index]) {
            memcpy(&return_value, &read_page_table[index][offset], sizeof(T));
        } else {
            if constexpr (sizeof(T) == 1) {
                return ReadByte(addr);
            } else if constexpr (sizeof(T) == 2) {
                return ReadHalf(addr);
            } else {
                return ReadWord(addr);
            }
        }

        return return_value;
    }

    template <typename T>
    void FastWrite(u32 addr, T data) {
        addr &= ~(sizeof(T) - 1);

        int index = addr >> 12;
        int offset = addr & 0xFFF;

        if (write_page_table[index]) {
            memcpy(&write_page_table[index][offset], &data, sizeof(T));
        } else {
            if constexpr (sizeof(T) == 1) {
                WriteByte(addr, data);
            } else if constexpr (sizeof(T) == 2) {
                WriteHalf(addr, data);
            } else {
                WriteWord(addr, data);
            }
        }
    }

    template <int size>
    std::array<u8, size> LoadBios(std::string path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::array<u8, size> bios;

        if (!file) {
            log_fatal("bios could not be found");
        }

        file.unsetf(std::ios::skipws);

        std::streampos bios_size = file.tellg();

        assert(bios_size <= size);

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(bios.data()), size);
        file.close();

        log_debug("bios loaded successfully!");

        return bios;
    }

    std::array<u8*, 0x100000> read_page_table;
    std::array<u8*, 0x100000> write_page_table;

    template <typename T>
    using ReadCallback = Common::Callback<T(u32)>;

    template <typename T>
    using WriteCallback = Common::Callback<void(u32, T)>;

    template <typename T>
    struct ReadHandler {
        ReadCallback<T> callback;
        bool mapped = false;
    };

    template <typename T>
    struct WriteHandler {
        WriteCallback<T> callback;
        bool mapped = false;
    };

    template <typename T, int N>
    using ReadHandlers = std::array<ReadHandler<T>, N / sizeof(T)>;

    template <typename T, int N>
    using WriteHandlers = std::array<WriteHandler<T>, N / sizeof(T)>;

    template <typename T>
    ReadCallback<T> invalid_read() {
        return [](u32 addr) -> T {
            log_fatal("invalid read %08x", addr);
        };
    }

    template <typename T>
    ReadCallback<T> direct_read(T* mmio, u32 mask) {
        return [&](u32) -> T {
            return *mmio & mask;
        };
    }

    // just fallthrough
    template <typename T>
    ReadCallback<T> complex_read(ReadCallback<T> callback) {
        return callback;
    }

    // just fallthrough
    template <typename T>
    WriteCallback<T> complex_write(WriteCallback<T> callback) {
        return callback;
    }

private:
    virtual auto ReadByte(u32 addr) -> u8 = 0;
    virtual auto ReadHalf(u32 addr) -> u16 = 0;
    virtual auto ReadWord(u32 addr) -> u32 = 0;

    virtual void WriteByte(u32 addr, u8 data) = 0;
    virtual void WriteHalf(u32 addr, u16 data) = 0;
    virtual void WriteWord(u32 addr, u32 data) = 0;
};