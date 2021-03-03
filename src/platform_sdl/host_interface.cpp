#include "host_interface.h"

HostInterface::HostInterface() {
    core = std::make_unique<Core>();
}

bool HostInterface::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_warn("error initialising SDL!");
        return false;
    }

    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    int window_size = 2;

    window = SDL_CreateWindow("yuugen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * window_size, 384 * window_size, window_flags);

    if (window == NULL) {
        log_warn("error initialising SDL_Window!");
        return false;
    }

    // create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        log_warn("error initialising SDL_Renderer");
        return false;
    }

    // create the top and bottom textures
    top_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
    bottom_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 192);

    if (top_texture == NULL) {
        log_warn("error initialising SDL_Texture for the top screen");
        return false;
    }

    if (bottom_texture == NULL) {
        log_warn("error initialising SDL_Texture for the bottom screen");
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

void HostInterface::Run(const char* path) {
    core->SetRomPath(path);
    core->Reset();
    core->DirectBoot();

    u32 frame_time_start = SDL_GetTicks();
    while (true) {
        core->RunFrame();

        SDL_UpdateTexture(top_texture, nullptr, core->gpu.GetFramebuffer(Screen::TOP_SCREEN), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_texture, nullptr, core->gpu.GetFramebuffer(Screen::BOTTOM_SCREEN), sizeof(u32) * 256);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, top_texture, nullptr, &top_texture_area);
        SDL_RenderCopy(renderer, bottom_texture, nullptr, &bottom_texture_area);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                Cleanup();
                return;
            }
            if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
                bool key_pressed = event.type == SDL_KEYDOWN;
                switch (event.key.keysym.sym) {
                case SDLK_d:
                    // A
                    core->input.HandleInput(BUTTON_A, key_pressed);
                    break;
                case SDLK_s:
                    // B
                    core->input.HandleInput(BUTTON_B, key_pressed);
                    break;
                // should handle X and Y later (not in keyinput)
                case SDLK_RSHIFT:
                    // select
                    core->input.HandleInput(BUTTON_SELECT, key_pressed);
                    break;
                case SDLK_RETURN:
                    // start
                    core->input.HandleInput(BUTTON_START, key_pressed);
                    break;
                case SDLK_RIGHT:
                    // right
                    core->input.HandleInput(BUTTON_RIGHT, key_pressed);
                    break;
                case SDLK_LEFT:
                    // left 
                    core->input.HandleInput(BUTTON_LEFT, key_pressed);
                    break;
                case SDLK_UP:
                    // up
                    core->input.HandleInput(BUTTON_UP, key_pressed);
                    break;
                case SDLK_DOWN:
                    // down
                    core->input.HandleInput(BUTTON_DOWN, key_pressed);
                    break;
                case SDLK_e:
                    // Button R
                    core->input.HandleInput(BUTTON_R, key_pressed);
                    break;
                case SDLK_w:
                    // Button L
                    core->input.HandleInput(BUTTON_L, key_pressed);
                    break;
                }
            }
        }
        u32 frame_time_end = SDL_GetTicks();
        frames++;
        if ((frame_time_end - frame_time_start) >= 1000) {
            snprintf(window_title, 30, "yuugen - %d FPS", frames);
            SDL_SetWindowTitle(window, window_title);
            frame_time_start = SDL_GetTicks();
            frames = 0;
        }
    }
    
    
}

void HostInterface::Cleanup() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(top_texture);
    SDL_DestroyTexture(bottom_texture);
    SDL_Quit();
}
