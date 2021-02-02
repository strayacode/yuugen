#pragma once


#include <common/types.h>
#include <common/log.h>
#include <common/arithmetic.h>
#include <nds/cartridge.h>
#include <stdio.h>
#include <string>

class NDS;

class NDS_Cartridge {
public:
	NDS_Cartridge(NDS *nds);
	~NDS_Cartridge();

	void write_romctrl(u32 value);
	void write_auxspicnt(u16 value);
	void write_auxspidata(u16 value);
	void write_hi_auxspicnt(u16 value);

	// dont worry about encryption stuff in rom ctrl for now
	// if bit 31 gets set in a write then a cartridge command occurs
	u32 ROMCTRL;

	u8 command_buffer[8] = {};

	// this stores data that has been retrieved by cartridge commands
	u32 data_output;

	u8 *rom;

	void do_command();
	void load_cartridge(std::string rom_path);
	// used in direct boot
	void transfer_rom();

	struct cartridge_header {
        char game_title[12];
        u32 gamecode; // this is 0 on homebrew
        u16 makercode; // this is 0 on homebrew and 01 on nintendo

        u32 arm9_rom_offset; // specifies the offset in rom that data will start to be transferred to main ram
        u32 arm9_entry_address; // specifies what r15 will be set to on direct boot
        u32 arm9_ram_address; // specifies the address in the arm9 bus that data will start to be transferred to
        u32 arm9_size; // specifies the size of data being transferred to main ram
        u32 arm7_rom_offset; // same for arm7
        u32 arm7_entry_address;
        u32 arm7_ram_address;
        u32 arm7_size;
    } header;
    
private:
	NDS *nds;

	void load_header_data();

	u16 AUXSPICNT;
	u16 AUXSPIDATA;


	bool key1_encryption = false;
	
};