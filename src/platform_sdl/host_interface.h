#pragma once

#include <SDL2/SDL.h>
#include <common/types.h>
#include <nds/nds.h>
#include <stdio.h>
#include <string>

class HostInterface {
public:
	void run(std::string rom_path);
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

	NDS nds;

	

	bool initialise_nds();
	bool initialise_gba();

	void run_nds(std::string rom_path);
	void run_gba(std::string rom_path);
};