#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <common/types.h>

class EmuCore {
public:
	EmuCore() {};
	~EmuCore() {};

	// these class methods are common between the GBA and NDS class which allows us to use EmuCore as the class type for both
	virtual void direct_boot(std::string rom_path) = 0;
	virtual void run_frame() = 0;
	virtual void reset() = 0;

	virtual const u32* get_framebuffer(int screen) = 0;

	virtual void handle_input() = 0;

	// we will use SDL_Event even in qt as sdl provides support for input from controllers
	SDL_Event event;

	enum screens {
		TOP_SCREEN,
		BOTTOM_SCREEN,
	};

	bool running = true;
private:
};