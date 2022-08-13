#pragma once

#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/Callback.h"
#include "Core/arm/ARMTypes.h"

class MMIO {
public:
    MMIO(Arch arch) : arch(arch) {}

    template <typename T>
    using ReadCallback = Common::Callback<T(u32)>;

    template <typename T>
    using WriteCallback = Common::Callback<void(u32, T)>;

    template <typename T>
    T read(u32 addr) {
        auto handler = get_read_handler<T>(addr);

        if (!handler.mapped) {
            log_fatal("[MMIO] handle unmapped %s %lu-bit read %08x", get_arch(arch).c_str(), sizeof(T) * 8, addr);
        }

        return handler.callback(addr);
    }

    template <typename T>
    void write(u32 addr, T data) {
        auto handler = get_write_handler<T>(addr);

        if (!handler.mapped) {
            log_fatal("[MMIO] handle unmapped %s %lu-bit write %08x", get_arch(arch).c_str(), sizeof(T) * 8, addr);
        }

        handler.callback(addr, data);
    }

    template <typename T>
    void register_mmio(u32 addr, ReadCallback<T> read_callback, WriteCallback<T> write_callback) {
        ReadHandler<T>& read_handler = get_read_handler<T>(addr);
        WriteHandler<T>& write_handler = get_write_handler<T>(addr);

        read_handler.callback = read_callback;
        read_handler.mapped = true;

        write_handler.callback = write_callback;
        write_handler.mapped = true;
    }

    template <typename T>
    ReadCallback<T> invalid_read() {
        return [](u32 addr) -> T {
            log_fatal("invalid read %08x", addr);
        };
    }

    template <typename T, typename BackingT = T>
    ReadCallback<T> direct_read(BackingT* mmio, u32 mask = 0xFFFFFFFF) {
        return [mmio, mask](u32) -> T {
            return *mmio & mask;
        };
    }

    // just fallthrough
    template <typename T>
    ReadCallback<T> complex_read(ReadCallback<T> callback) {
        return callback;
    }

    template <typename T>
    WriteCallback<T> invalid_write() {
        return [](u32 addr, T data) {
            log_fatal("invalid write %08x = %08x", addr, data);
        };
    }

    template <typename T, typename BackingT = T>
    WriteCallback<T> direct_write(BackingT* mmio, u32 mask = 0xFFFFFFFF) {
        return [mmio, mask](u32, T data) {
            *mmio = data & mask;
        };
    }

    // just fallthrough
    template <typename T>
    WriteCallback<T> complex_write(WriteCallback<T> callback) {
        return callback;
    }

private:
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

    template <typename T>
    ReadHandler<T>& get_read_handler(u32 addr) {
        addr -= 0x04000000;

        if constexpr (sizeof(T) == 1) {
            return read8[addr];
        } else if constexpr (sizeof(T) == 2) {
            return read16[addr];
        } else {
            return read32[addr];
        }
    }

    template <typename T>
    WriteHandler<T>& get_write_handler(u32 addr) {
        addr -= 0x04000000;

        if constexpr (sizeof(T) == 1) {
            return write8[addr];
        } else if constexpr (sizeof(T) == 2) {
            return write16[addr];
        } else {
            return write32[addr];
        }
    }

    static constexpr int NUM_MMIOS = 0x1080;

    template <typename T>
    using ReadHandlers = std::array<ReadHandler<T>, NUM_MMIOS / sizeof(T)>;

    template <typename T>
    using WriteHandlers = std::array<WriteHandler<T>, NUM_MMIOS / sizeof(T)>;

    ReadHandlers<u8> read8;
    ReadHandlers<u16> read16;
    ReadHandlers<u32> read32;
    WriteHandlers<u8> write8;
    WriteHandlers<u16> write16;
    WriteHandlers<u32> write32;

    Arch arch;
};