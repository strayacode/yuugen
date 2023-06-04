#pragma once

#include "common/types.h"

namespace core {

class SPI {
public:
    void reset();

    u16 read_spicnt() { return spicnt; }
    u8 read_spidata() { return spidata; }

private:
    u16 spicnt;
    u8 spidata;
};

} // namespace core