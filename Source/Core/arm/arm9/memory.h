#pragma once

#include "Common/Types.h"
#include "Core/arm/memory_base.h"

class System;

class ARM9Memory : public MemoryBase {
public:
    ARM9Memory(System& system);

    void Reset();
    void UpdateMemoryMap(u32 low_addr, u32 high_addr);

    u8 ReadByte(u32 addr) override;
    u16 ReadHalf(u32 addr) override;
    u32 ReadWord(u32 addr) override;

    template <typename T>
    T mmio_read(u32 addr) {
        auto handler = get_read_handler<T>(addr);

        if (!handler.mapped) {
            log_fatal("[ARM9Memory] handle unmapped %lu-bit read %08x", sizeof(T) * 8, addr);
        }

        return handler.callback(addr);
    }

    template <typename T>
    void mmio_write(u32 addr, T data) {
        auto handler = get_write_handler<T>(addr);

        if (!handler.mapped) {
            log_fatal("[ARM9Memory] handle unmapped %lu-bit write %08x", sizeof(T) * 8, addr);
        }

        handler.callback(addr, data);
    }

    void WriteByte(u32 addr, u8 data) override;
    void WriteHalf(u32 addr, u16 data) override;
    void WriteWord(u32 addr, u32 data) override;

private:
    System& system;
    std::array<u8, 0x8000> bios;

    static constexpr int NUM_MMIOS = 0x1080;

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

    template <typename T>
    void register_mmio(u32 addr, ReadCallback<T> read_callback, WriteCallback<T> write_callback);

    void build_mmio();

    ReadHandlers<u8, NUM_MMIOS> read8;
    ReadHandlers<u16, NUM_MMIOS> read16;
    ReadHandlers<u32, NUM_MMIOS> read32;
    WriteHandlers<u8, NUM_MMIOS> write8;
    WriteHandlers<u16, NUM_MMIOS> write16;
    WriteHandlers<u32, NUM_MMIOS> write32;
};