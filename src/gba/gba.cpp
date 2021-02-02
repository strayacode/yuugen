#include <gba/gba.h>

void GBA::direct_boot(std::string rom_path) {
	cartridge.load_cartridge(rom_path);

	arm7.direct_boot();
}

void GBA::run_frame() {

}


void GBA::reset() {

}

const u32* GBA::get_framebuffer(int screen) {

}

void GBA::handle_input() {
	while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
            bool key_pressed = event.type == SDL_KEYDOWN;
            switch (event.key.keysym.sym) {
            case SDLK_u:
                // A
                input.handle_keypress(0, key_pressed);
                break;
            case SDLK_i:
                // B
                input.handle_keypress(1, key_pressed);
                break;
            // should handle X and Y later (not in keyinput)
            case SDLK_RSHIFT:
                // select
                input.handle_keypress(2, key_pressed);
                break;
            case SDLK_RETURN:
                // start
                input.handle_keypress(3, key_pressed);
                break;
            case SDLK_RIGHT:
                // right
                input.handle_keypress(4, key_pressed);
                break;
            case SDLK_LEFT:
                // left 
                input.handle_keypress(5, key_pressed);
                break;
            case SDLK_UP:
                // up
                input.handle_keypress(6, key_pressed);
                break;
            case SDLK_DOWN:
                // down
                input.handle_keypress(7, key_pressed);
                break;
            case SDLK_e:
                // Button R
                input.handle_keypress(8, key_pressed);
                break;
            case SDLK_q:
                // Button L
                input.handle_keypress(9, key_pressed);
                break;

            }
        }
    }
}