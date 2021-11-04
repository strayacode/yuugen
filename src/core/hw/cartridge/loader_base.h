#pragma once

#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <common/types.h>
#include <common/log.h>

class LoaderBase {
public:
    LoaderBase() {
        Reset();
    }

    void Reset() {
        rom.clear();
        rom_size = 0;
        backup_size = 0;
        backup_type = 0;
    }

    void Load(std::string path) {
        if (path == "") {
            backup_size = 0;
            return;
        }

        rom_path = path;

        std::ifstream file(rom_path, std::ios::binary);

        if (!file) {
            log_fatal("rom with path %s does not exist!", rom_path.c_str());
        }

        file.unsetf(std::ios::skipws);
        file.seekg(0, std::ios::end);

        rom_size = file.tellg();

        file.seekg(0, std::ios::beg);
        rom.reserve(rom_size);
        rom.insert(rom.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

        file.close();

        log_debug("rom data loaded");
        log_debug("Size: %08lx", rom_size);

        LoadHeader();
        LoadBackup();
    }

    void SetPath(std::string path) {
        rom_path = path;
    }

    // returns a pointer to a certain offset in the rom
    u8* GetPointer(u32 offset) {
        return rom.data() + offset;
    }

    u64 GetSize() {
        return rom_size;
    }

    u32 GetBackupSize() {
        return backup_size;
    }

    virtual void LoadHeader() = 0;
    virtual void LoadBackup() = 0;
// private:
    std::vector<u8> rom;
    u64 rom_size;
    u32 backup_type;
    u32 backup_size;
    std::string rom_path;
};