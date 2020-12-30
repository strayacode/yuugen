#include <emulator/emulator.h>
#include <stdio.h>
#include <string>
#include <SDL2/SDL.h>

Emulator::Emulator() : arm9(this, 1), arm7(this, 0), memory(this), cartridge(this), gpu(this), dma {DMA(this, 0), DMA(this, 1)} {
    // set to regular size
    window_size_multiplier = 2;
    sdl_init();
}

void Emulator::reset() {
    running = true;
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    memory.load_firmware();
    arm9.reset();
    arm7.reset();
}

void Emulator::run_nds_frame() {
    // quick sidenote
    // in 1 frame of the nds executing
    // there are 263 scanlines with 192 visible and 71 for vblank
    // in each scanline there are 355 dots in total with 256 visible and 99 for hblank
    // 3 cycles of the arm7 occurs per dot and 6 cycles of the arm9 occurs per dot
    for (int i = 0; i < 263; i++) {
        for (int j = 0; j < 355 * 3; j++) {
            // run arm9 and arm7 stuff
            // since arm9 runs at the twice the clock speed of arm7 we run it 2 times instead of 1
            
            arm9.step();
            arm9.step();
            arm7.step();
            
        }
        gpu.render_scanline(i);
    }
}

void Emulator::run(std::string rom_path) {
    cartridge.load_cartridge(rom_path);
    
    cartridge.direct_boot();
    cp15.direct_boot();
    
    reset();
    
    while (running) {
        // call run nds frame
        // run_nds_frame();
        run_nds_frame();
        // update textures
        SDL_UpdateTexture(top_texture, nullptr, gpu.get_framebuffer(gpu.top_screen), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_texture, nullptr, gpu.get_framebuffer(gpu.bottom_screen), sizeof(u32) * 256);
        // clear and copy texture into renderer
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, top_texture, nullptr, &top_texture_dimensions);
        SDL_RenderCopy(renderer, bottom_texture, nullptr, &bottom_texture_dimensions);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);

        // check for events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                // cool use of goto lol
                goto cleanup;
            }
        }
        
    }

    cleanup:
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(top_texture);
        SDL_DestroyTexture(bottom_texture);
        SDL_Quit();
}

void Emulator::sdl_init() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("[Emulator] error while initialising SDL!\n");
        running = false;
        return;
    }

    // setup basic stuff
    window = SDL_CreateWindow("ChronoDS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256 * window_size_multiplier, 384 * window_size_multiplier, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    top_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    bottom_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    // setup top and bottom texture dimensions
    top_texture_dimensions.x = 0;
    top_texture_dimensions.y = 0;
    top_texture_dimensions.w = 256 * window_size_multiplier;
    top_texture_dimensions.h = 192 * window_size_multiplier;

    bottom_texture_dimensions.x = 0;
    bottom_texture_dimensions.y = 192 * window_size_multiplier;
    bottom_texture_dimensions.w = 256 * window_size_multiplier;
    bottom_texture_dimensions.h = 192 * window_size_multiplier;
}