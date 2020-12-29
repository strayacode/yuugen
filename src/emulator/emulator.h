#pragma once
#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/core/memory.h>
#include <emulator/core/cartridge.h>
#include <emulator/core/cp15.h>
#include <emulator/core/gpu.h>
#include <emulator/core/dma.h>
#include <emulator/core/interrupt.h>
#include <string>
#include <SDL2/SDL.h>

class Emulator {
public:
    Emulator();
    void run_nds_frame();
    void reset();
    void run(std::string rom_path);
    ARM arm9;
    ARM arm7;
    Memory memory;
    Cartridge cartridge;
    CP15 cp15;
    GPU gpu;
    DMA dma[2];
    Interrupt interrupt;

    bool running;

    


private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *top_texture;
    SDL_Texture *bottom_texture;
    SDL_Rect top_texture_dimensions;
    SDL_Rect bottom_texture_dimensions;

    // Event handler
    SDL_Event e;

    int window_size_multiplier;

    void sdl_init(); // initialises sdl stuff
};