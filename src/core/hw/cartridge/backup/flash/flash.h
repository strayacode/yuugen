#pragma once

#include <memory>
#include <string>
#include <common/backup.h>
#include <common/types.h>

enum class BackupSize {
    SIZE_256K,
    SIZE_512K,
    SIZE_1024K,
    SIZE_8192K,
};

struct FlashBackup {
    FlashBackup(std::string path, u32 size);

    std::string path;

    u32 size;

    std::unique_ptr<Backup> backup;
};