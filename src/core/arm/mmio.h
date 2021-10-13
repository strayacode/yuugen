#pragma once

#include <common/types.h>
#include <common/log.h>
#include <unordered_map>
#include <functional>

enum class MMIOType {
    ARMv4,
    ARMv5,
};

// this class is used to handle mmio accesses for either the arm7 or arm9.
// it also allows us to skip the slow path in memory accesses (slight performance gain)
class MMIO {
public:
    MMIO() {};

    template <typename T>
    void Register(u32 addr, std::function<T(u32)> read, std::function<void(u32, T)> write) {
        switch (sizeof(T)) {
        case 1:
            read_handler8[addr] = read;
            write_handler8[addr] = write;
            break;
        case 2:
            read_handler16[addr] = read;
            write_handler16[addr] = write;
            break;
        case 4:
            read_handler32[addr] = read;
            write_handler32[addr] = write;
            break;
        }
    }

    template <typename T>
    T Read(u32 addr) {
        std::function<T(u32)> handler = GetReadHandler<T>(addr);

        if (handler) {
            return handler(addr);
        } else {
            log_fatal("[MMIO] Handle %ld-bit mmio read at address %08x", sizeof(T) * 8, addr);
        }
    }

    template <typename T>
    void Write(u32 addr, T data) {
        std::function<void(u32, T)> handler = GetWriteHandler<T>(addr);
        
        if (handler) {
            handler(addr, data);
        } else {
            log_fatal("[MMIO] Handle %ld-bit mmio write at address %08x = %08x", sizeof(T) * 8, addr, data);
        }
    }

    template <typename T>
    std::function<T(u32)> GetReadHandler(u32 addr) {
        switch (sizeof(T)) {
        case 1:
            return read_handler8[addr];
        case 2:
            return read_handler16[addr];
        case 4:
            return read_handler32[addr];
        }
    }

    template <typename T>
    std::function<void(u32, T)> GetWriteHandler(u32 addr) {
        switch (sizeof(T)) {
        case 1:
            return write_handler8[addr];
        case 2:
            return write_handler16[addr];
        case 4:
            return write_handler32[addr];
        }
    }

private:
    template <typename T>
    struct Handler {
        using Read = std::function<T(u32)>;
        using Write = std::function<void(u32, T)>;
    };

    std::unordered_map<u32, Handler<u8>::Read> read_handler8;
    std::unordered_map<u32, Handler<u16>::Read> read_handler16;
    std::unordered_map<u32, Handler<u32>::Read> read_handler32;

    std::unordered_map<u32, Handler<u8>::Write> write_handler8;
    std::unordered_map<u32, Handler<u16>::Write> write_handler16;
    std::unordered_map<u32, Handler<u32>::Write> write_handler32;
};