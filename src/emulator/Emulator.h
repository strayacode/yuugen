#pragma once
#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/core/Memory.h>
#include <emulator/core/Cartridge.h>
#include <emulator/core/CP15.h>
#include <emulator/core/GPU.h>
#include <emulator/core/DMA.h>
#include <emulator/core/Interrupt.h>
#include <emulator/core/Keypad.h>
#include <string>
#include <SDL2/SDL.h>

class Emulator {
public:
    Emulator();
    
    void reset();
    ARMInterpreter arm9;
    ARMInterpreter arm7;
    Memory memory;
    Cartridge cartridge;
    CP15 cp15;
    GPU gpu;
    DMA dma[2];
    Interrupt interrupt;
    Keypad keypad;

    bool running;
    int frames = 0; // measures fps
    void run_nds_frame();

    char window_title[30];
    int window_size_multiplier;
private:
    

    

    
};