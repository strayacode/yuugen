#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <string>
#include "Common/Log.h"
#include "Common/Types.h"

// a class which provides functionality to have a file memory mapped.
// this allows the kernel to only read data from a file when it's required,
// which avoids the long loading times required for games like Pokemon B2/W2
// which are 512mb in size
class MemoryMappedFile {
public:
    MemoryMappedFile() {
    #ifdef _WIN32
        log_fatal("yuugen hasn't been used/tested for windows yet");
    #endif
    }

    ~MemoryMappedFile() {
        // is this correct?
        if (buffer != nullptr) {
            munmap(buffer, 4096);
        }
    }

    void Load(std::string path) {
        fd = open(path.c_str(), O_RDONLY, 0);

        size = lseek(fd, 0, SEEK_END);
        buffer = reinterpret_cast<u8*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0));
    }

    u8* GetPointer(u32 offset) {
        if (buffer == nullptr) {
            log_fatal("file is not mapped yet!");
        }

        return buffer + offset;
    }

    u64 GetSize() {
        return size;
    }

private:
    int fd;
    u8* buffer = nullptr;
    u64 size = 0;
};