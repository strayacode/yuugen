#include "host_interface.h"
#include <stdio.h>


bool HostInterface::initialise() {
	// initialise sdl video
	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		printf("error initialising SDL!");
		return false;
	}
	SDL_Init(SDL_INIT_VIDEO);

	// set the window size multiplier
	window_size = 2;


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

void HostInterface::cleanup() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(top_screen_texture);
	SDL_DestroyTexture(bottom_screen_texture);
	SDL_Quit();
}

void HostInterface::run(std::string rom_path) {
	nds.firmware_boot();

	u32 frame_time_start = SDL_GetTicks();
	while (true) {
		nds.run_nds_frame();

		SDL_UpdateTexture(top_screen_texture, nullptr, nds.gpu.get_framebuffer(nds.gpu.TOP_SCREEN), sizeof(u32) * 256);
        SDL_UpdateTexture(bottom_screen_texture, nullptr, nds.gpu.get_framebuffer(nds.gpu.BOTTOM_SCREEN), sizeof(u32) * 256);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, top_screen_texture, nullptr, &top_texture_area);
        SDL_RenderCopy(renderer, bottom_screen_texture, nullptr, &bottom_texture_area);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                cleanup();
                return;
            }
            if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
                bool key_pressed = event.type == SDL_KEYDOWN;
                switch (event.key.keysym.sym) {
                case SDLK_u:
                    // A
                    nds.input.handle_keypress(0, key_pressed);
                    break;
                case SDLK_i:
                    // B
                    nds.input.handle_keypress(1, key_pressed);
                    break;
                // should handle X and Y later (not in keyinput)
                case SDLK_RSHIFT:
                    // select
                    nds.input.handle_keypress(2, key_pressed);
                    break;
                case SDLK_RETURN:
                    // start
                    nds.input.handle_keypress(3, key_pressed);
                    break;
                case SDLK_RIGHT:
                    // right
                    nds.input.handle_keypress(4, key_pressed);
                    break;
                case SDLK_LEFT:
                    // left 
                    nds.input.handle_keypress(5, key_pressed);
                    break;
                case SDLK_UP:
                    // up
                    nds.input.handle_keypress(6, key_pressed);
                    break;
                case SDLK_DOWN:
                    // down
                    nds.input.handle_keypress(7, key_pressed);
                    break;
                case SDLK_e:
                    // Button R
                    nds.input.handle_keypress(8, key_pressed);
                    break;
                case SDLK_q:
                    // Button L
                    nds.input.handle_keypress(9, key_pressed);
                    break;

                }
            }
        }
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
}