#pragma once

#include <common/types.h>
#include <common/log.h>
#include <stdio.h>

class NDS;

class SPI {
public:
	SPI(NDS *nds);

	u16 SPICNT;

	u8 SPIDATA;

	void write_spicnt(u16 data);
	void write_spidata(u8 data);
	void load_firmware();

    // used to load the first 0x70 bytes of user settings area 2 into main memory starting from address 0x027FFC80
    void direct_boot();
private:
	NDS *nds;

	// external to the arm7/arm9 bus
    u8 firmware[0x40000] = {}; // built-in serial flash memory

    // data corresponds to command
    u8 firmware_transfer(u8 command);

    u8 command_id = 0;

    // ig this is used to keep track of the current address to read from???
    u32 address = 0;

    // seems like this is used to keep track of if a chip is still selected or not
    // if write count is 0 then new command must be interpreted
    int write_count = 0;


    enum command_ids {
    	COMMAND_NONE = 0x00,
    	READ_STREAM = 0x03,
    };
};