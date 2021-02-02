#pragma once

#include <SDL2/SDL.h>
#include <common/types.h>
#include <common/emu_core.h>
#include <nds/nds.h>
#include <gba/gba.h>
#include <stdio.h>
#include <string>
#include <memory>

class HostInterface {
public:
	void run_core(std::string rom_path);
	// use bool so that we can return true or false whether the initialisation process was successful or not and exit efficiently 
	bool initialise();
	
	void cleanup();

	void set_rom_type(std::string rom_path);
private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* top_screen_texture = nullptr;
	SDL_Texture* bottom_screen_texture = nullptr;

	SDL_Rect top_texture_area;
    SDL_Rect bottom_texture_area;


	SDL_Event event;

	int frames = 0;

	int window_size;

	char window_title[30];

	enum rom_types {
		NDS_ROM,
		GBA_ROM,
	};

	int rom_type;

	std::unique_ptr<EmuCore> emu_core;

	bool initialise_nds();
	bool initialise_gba();
};