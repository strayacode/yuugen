#pragma once

#include <common/types.h>

// a simple struct used to hold data about a certain block
struct Block {
    // used to detect whether a block is executing 
    // a block is pretty much always going to run in one arm mode.
    // if for example the block is thumb code but later when the block is executed
    // again the cpu is in arm mode, then all the instructions would be different which i doubt is ever used
    bool arm;

    // used for blocks which can be overwritten
    bool writeable;

    // represents the starting address of the block in the address space
    u32 address;

    // a pointer to the start of the code. the idea is it points
    // to an instruction struct
    u8* code_entry;

    int num_instructions;
};