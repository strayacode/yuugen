#pragma once


#include <fstream>
#include <vector>
#include <string>
#include <iterator>
#include <memory>
#include <string.h>
#include "common/types.h"
#include "common/log.h"

class GBACartridge {
public:
    void Reset();
    void LoadRom(std::string path);
    void LoadHeaderData();
    u64 GetRomSize();

    std::vector<u8> rom;

    u64 rom_size;
};