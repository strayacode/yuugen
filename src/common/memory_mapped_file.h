#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <string>
#include "common/logger.h"
#include "common/types.h"

namespace common {

class MemoryMappedFile {
public:
    MemoryMappedFile() {
#ifdef _WIN32
        LOG_ERROR("yuugen hasn't been used/tested for windows yet");
#endif
    }

    ~MemoryMappedFile() {
        // is this correct?
        if (buffer != nullptr) {
            munmap(buffer, 4096);
        }
    }

    void load(const std::string& path) {
        fd = open(path.c_str(), O_RDONLY, 0);
        size = lseek(fd, 0, SEEK_END);
        buffer = reinterpret_cast<u8*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0));
    }

    u8* get_pointer(u32 offset) {
        if (buffer == nullptr) {
            LOG_ERROR("file is not mapped yet!");
        }

        return buffer + offset;
    }

    u64 get_size() {
        return size;
    }

private:
    int fd;
    u8* buffer = nullptr;
    u64 size = 0;
};

} // namespace common