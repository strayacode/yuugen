#include <emulator/Emulator.h>
#include <SDL2/SDL.h>
#include <emulator/common/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>

void main_loop(std::string rom_path, Emulator* emulator) {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_fatal("error while initialising SDL!");
    }

    

    SDL_Window* window = SDL_CreateWindow("ChronoDS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 384, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* top_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    SDL_Texture* bottom_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    SDL_Event event;


    SDL_Rect top_texture_area = {
        .x = 0,
        .y = 0,
        .w = 256,
        .h = 192,
    };
    SDL_Rect bottom_texture_area {
        .x = 0,
        .y = 192,
        .w = 256,
        .h = 192,
    };

    emulator->cartridge.load_cartridge(rom_path);

    emulator->cartridge.direct_boot();
    emulator->cp15.direct_boot();
    emulator->reset();

    u32 frame_time_start = SDL_GetTicks();
    while (emulator->running) {
        emulator->run_nds_frame();

        SDL_UpdateTexture(top_texture, nullptr, emulator->gpu.get_framebuffer(emulator->gpu.top_screen), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_texture, nullptr, emulator->gpu.get_framebuffer(emulator->gpu.bottom_screen), sizeof(u32) * 256);
        // clear and copy texture into renderer
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, top_texture, nullptr, &top_texture_area);
        SDL_RenderCopy(renderer, bottom_texture, nullptr, &bottom_texture_area);
        SDL_RenderPresent(renderer);
        // check for events
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                // cool use of goto lol
                goto cleanup;
            }
            if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
                bool key_pressed = event.type == SDL_KEYDOWN;
                switch (event.key.keysym.sym) {
                case SDLK_u:
                    // A
                    emulator->keypad.handle_key(0, key_pressed);
                    break;
                case SDLK_i:
                    // B
                    emulator->keypad.handle_key(1, key_pressed);
                    break;
                // should handle X and Y later (not in keyinput)
                case SDLK_RSHIFT:
                    // select
                    emulator->keypad.handle_key(2, key_pressed);
                    break;
                case SDLK_RETURN:
                    // start
                    emulator->keypad.handle_key(3, key_pressed);
                    break;
                case SDLK_RIGHT:
                    // right
                    emulator->keypad.handle_key(4, key_pressed);
                    break;
                case SDLK_LEFT:
                    // left 
                    emulator->keypad.handle_key(5, key_pressed);
                    break;
                case SDLK_UP:
                    // up
                    emulator->keypad.handle_key(6, key_pressed);
                    break;
                case SDLK_DOWN:
                    // down
                    emulator->keypad.handle_key(7, key_pressed);
                    break;
                case SDLK_e:
                    // Button R
                    emulator->keypad.handle_key(8, key_pressed);
                    break;
                case SDLK_q:
                    // Button L
                    emulator->keypad.handle_key(9, key_pressed);
                    break;

                }
            }
        }
        u32 frame_time_end = SDL_GetTicks();
        emulator->frames++;
        SDL_Delay(16);
        if (frame_time_end - frame_time_start >= 1000) {
            snprintf(emulator->window_title, 30, "ChronoDS - %d FPS", emulator->frames);
            // log_debug("frames is %d fps", frames);
            SDL_SetWindowTitle(window, emulator->window_title);
            frame_time_start = SDL_GetTicks();
            emulator->frames = 0;
        }
    }
    cleanup:
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(top_texture);
        SDL_DestroyTexture(bottom_texture);
        SDL_Quit();
    
}


int main(int argc, char *argv[]) {
    std::unique_ptr<Emulator> emulator = std::make_unique<Emulator>();
    if (argc < 2) {
        log_fatal("no rom argument or other arguments were specified!\n");
    }
    int i;
    for (i = 1; i < argc - 1; i++) {
        printf("%s\n", argv[i]);
    }
    
    main_loop(argv[i], emulator.get());
    
    return 0;
}