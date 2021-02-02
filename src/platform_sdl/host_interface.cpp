#include "host_interface.h"
#include <stdio.h>


bool HostInterface::initialise() {
	if (rom_type == NDS_ROM) {
        return initialise_nds();
    } else if (rom_type == GBA_ROM) {
        return initialise_gba();
    } else {
        return false;
    }

    
}

bool HostInterface::initialise_nds() {
    // initialise sdl video
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("error initialising SDL!");
        return false;
    }
    SDL_Init(SDL_INIT_VIDEO);

    // set the window size multiplier
    window_size = 2;

    // set the core to nds
    emu_core = std::make_unique<NDS>();

    // create window
    // TODO: maybe add possibility for opengl context later?
    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    window = SDL_CreateWindow("ChronoDS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * window_size, 384 * window_size, window_flags);

    if (window == NULL) {
        printf("error initialising SDL_Window!\n");
        return false;
    }

    // create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("error initialising SDL_Renderer\n");
        return false;
    }

    // create the top and bottom textures
    top_screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    bottom_screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);

    if (top_screen_texture == NULL) {
        printf("error initialising SDL_Texture for the top screen\n");
        return false;
    }

    if (bottom_screen_texture == NULL) {
        printf("error initialising SDL_Texture for the bottom screen\n");
        return false;
    }

    top_texture_area = {
        .x = 0,
        .y = 0,
        .w = 256 * window_size,
        .h = 192 * window_size,
    };
    bottom_texture_area = {
        .x = 0,
        .y = 192 * window_size,
        .w = 256 * window_size,
        .h = 192 * window_size,
    };


    // if the function has reached this far it has successfully initialised
    return true;
}

bool HostInterface::initialise_gba() {
    // initialise sdl video
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("error initialising SDL!");
        return false;
    }
    SDL_Init(SDL_INIT_VIDEO);

    // set the window size multiplier
    window_size = 2;

    // set the core to gba
    emu_core = std::make_unique<GBA>();

    // create window
    // TODO: maybe add possibility for opengl context later?
    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    window = SDL_CreateWindow("ChronoGBA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 240 * window_size, 160 * window_size, window_flags);

    if (window == NULL) {
        printf("error initialising SDL_Window!\n");
        return false;
    }

    // create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("error initialising SDL_Renderer\n");
        return false;
    }

    // create the top and bottom textures
    top_screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 240, 160);

    // for gba bottom texture is meaningless

    if (top_screen_texture == NULL) {
        printf("error initialising SDL_Texture for the top screen\n");
        return false;
    }

    top_texture_area = {
        .x = 0,
        .y = 0,
        .w = 240 * window_size,
        .h = 160 * window_size,
    };
    


    // if the function has reached this far it has successfully initialised
    return true;
}

void HostInterface::cleanup() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(top_screen_texture);
	SDL_DestroyTexture(bottom_screen_texture);
	SDL_Quit();
}

void HostInterface::run_core(std::string rom_path) {
    emu_core->direct_boot(rom_path);

    u32 frame_time_start = SDL_GetTicks();
    while (true) {
        emu_core->run_frame();

        SDL_UpdateTexture(top_screen_texture, nullptr, emu_core->get_framebuffer(emu_core->TOP_SCREEN), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_screen_texture, nullptr, emu_core->get_framebuffer(emu_core->BOTTOM_SCREEN), sizeof(u32) * 256);
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, top_screen_texture, nullptr, &top_texture_area);
        SDL_RenderCopy(renderer, bottom_screen_texture, nullptr, &bottom_texture_area);
        SDL_RenderPresent(renderer);

        emu_core->handle_input();
        u32 frame_time_end = SDL_GetTicks();
        frames++;
        if (frame_time_end - frame_time_start >= 1000) {
            snprintf(window_title, 30, "ChronoDS - %d FPS", frames);
            // log_debug("frames is %d fps", frames);
            SDL_SetWindowTitle(window, window_title);
            frame_time_start = SDL_GetTicks();
            frames = 0;
        }
    }

    cleanup();
}

void HostInterface::set_rom_type(std::string rom_path) {
    if (rom_path.substr(rom_path.find_last_of(".") + 1) == "nds") {
        rom_type = NDS_ROM;
    } else {
        rom_type = GBA_ROM;
    }
}
