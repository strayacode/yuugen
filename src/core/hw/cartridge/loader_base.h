#pragma once

#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <common/types.h>
#include <common/log.h>
#include <common/memory_mapped_file.h>

class LoaderBase {
public:
    LoaderBase() {
        Reset();
    }

    void Reset() {
        backup_size = 0;
        backup_type = 0;
    }

    void Load(std::string path) {
        if (path == "") {
            backup_size = 0;
            return;
        }

        rom_path = path;

        memory_mapped_file.Load(rom_path);


        LoadHeader();
        LoadBackup();
    }

    void SetPath(std::string path) {
        rom_path = path;
    }

    // returns a pointer to a certain offset in the rom
    u8* GetPointer(u32 offset) {
        return memory_mapped_file.GetPointer(offset);
    }

    u64 GetSize() {
        return memory_mapped_file.GetSize();
    }

    u32 GetBackupSize() {
        return backup_size;
    }

    virtual void LoadHeader() = 0;
    virtual void LoadBackup() = 0;
// private:
    u32 backup_type;
    u32 backup_size;
    std::string rom_path;

    MemoryMappedFile memory_mapped_file;
};