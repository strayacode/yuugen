#pragma once
#include <emulator/common/types.h>

class Emulator;

typedef union {
    u16 raw;
    struct {
        u8 : 5;
        u8 dest_addr_control: 2;
        u8 source_addr_control: 2;
        u8 dma_repeat: 1;

    };
} dma_control_t;

class DMA {
public:
    DMA(Emulator *emulator, int dma_id);
    // 4 dma channels per cpu
    struct channel {
        u32 source_address; // only first 27 or 28 significant bits are used. i would this is the address that data is taken from
        u32 destination_address; // only first 27 or 28 bits are used. i would assume this specifies the address to copy data to
        u16 word_count; 
        u16 control; 
    } chan[4];

private:
    Emulator *emulator;
    int dma_id; // 1 = arm9, 0 = arm7
};