#include "host_interface.h"

HostInterface::HostInterface() : 
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
    
}

bool HostInterface::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_warn("error initialising SDL!");
        return false;
    }

    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    window_size = 2;

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

    top_texture_area.x = 0;
    top_texture_area.y = 0;
    top_texture_area.w = 256 * window_size;
    top_texture_area.h = 192 * window_size;
    
    bottom_texture_area.x = 0;
    bottom_texture_area.y = 192 * window_size;
    bottom_texture_area.w = 256 * window_size;
    bottom_texture_area.h = 192 * window_size;

    // initialise audio
    core.SetAudioInterface(audio_interface);

    // if the function has reached this far it has successfully initialised
    return true;
}

void HostInterface::Run(std::string path) {
    core.SetRomPath(path);
    core.SetBootMode(BootMode::Firmware);
    core.ToggleFramelimiter();
    core.SetState(State::Running);

    while (true) {
        SDL_UpdateTexture(top_texture, nullptr, core.hw.gpu.GetFramebuffer(TOP_SCREEN), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_texture, nullptr, core.hw.gpu.GetFramebuffer(BOTTOM_SCREEN), sizeof(u32) * 256);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, top_texture, nullptr, &top_texture_area);
        SDL_RenderCopy(renderer, bottom_texture, nullptr, &bottom_texture_area);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                Cleanup();
                return;
            } else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
                bool key_pressed = event.type == SDL_KEYDOWN;
                switch (event.key.keysym.sym) {
                case SDLK_d:
                    core.hw.input.HandleInput(BUTTON_A, key_pressed);
                    break;
                case SDLK_s:
                    core.hw.input.HandleInput(BUTTON_B, key_pressed);
                    break;
                case SDLK_RSHIFT:
                    core.hw.input.HandleInput(BUTTON_SELECT, key_pressed);
                    break;
                case SDLK_RETURN:
                    core.hw.input.HandleInput(BUTTON_START, key_pressed);
                    break;
                case SDLK_RIGHT:
                    core.hw.input.HandleInput(BUTTON_RIGHT, key_pressed);
                    break;
                case SDLK_LEFT:
                    core.hw.input.HandleInput(BUTTON_LEFT, key_pressed);
                    break;
                case SDLK_UP:
                    core.hw.input.HandleInput(BUTTON_UP, key_pressed);
                    break;
                case SDLK_DOWN:
                    core.hw.input.HandleInput(BUTTON_DOWN, key_pressed);
                    break;
                case SDLK_e:
                    core.hw.input.HandleInput(BUTTON_R, key_pressed);
                    break;
                case SDLK_w:
                    core.hw.input.HandleInput(BUTTON_L, key_pressed);
                    break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                int x = event.button.x / window_size;
                int y = event.button.y / window_size - 192;
                
                if ((y >= 0) && event.button.button == SDL_BUTTON_LEFT) {
                    // only do a touchscreen event if it occurs in the bottom screen
                    bool button_pressed = event.type == SDL_MOUSEBUTTONDOWN;
                    core.hw.input.SetTouch(button_pressed);
                    core.hw.input.SetPoint(x, y);
                }
            }
        }

        SDL_Delay(17);
    }
}

void HostInterface::UpdateTitle(float fps) {
    char window_title[40];
    snprintf(window_title, 40, "yuugen [%0.2f FPS | %0.2f ms]", fps, 1000.0 / fps);
    SDL_SetWindowTitle(window, window_title);
}

void HostInterface::Cleanup() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(top_texture);
    SDL_DestroyTexture(bottom_texture);
    SDL_Quit();
}
