#include <core/arm/arm7/memory.h>

auto ARM7Memory::ReadByte(u32 addr) -> u8 override {
    log_fatal("handle");
}

auto ARM7Memory::ReadHalf(u32 addr) -> u16 override {
    log_fatal("handle");
}

auto ARM7Memory::ReadWord(u32 addr) -> u32 override {
    log_fatal("handle");
}

void ARM7Memory::WriteByte(u32 addr, u8 data) override {
    log_fatal("handle");
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) override {
    log_fatal("handle");
}

void ARM7Memory::WriteWord(u32 addr, u32 data) override {
    log_fatal("handle");
}