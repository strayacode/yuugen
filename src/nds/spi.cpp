#include <nds/nds.h>
#include <nds/spi.h>


static const int FIRMWARE_SIZE = 256 * 1024;

SPI::SPI(NDS *nds) : nds(nds) {

}

void SPI::direct_boot() {
	for (int i = 0; i < 0x70; i++) {
		nds->memory.arm7_write_byte(0x027FFC80 + i, firmware[0x3FF00 + i]);
	}
}

void SPI::write_spicnt(u16 data) {
	SPICNT = data & 0xCF03;
}

void SPI::write_spidata(u8 data) {
	// on writes firmware transfer occurs

	// check if spi bus is actually enabled first
	if (!get_bit(15, SPICNT)) {
		SPIDATA = 0;
		return;
	}

	// this means a new command must be interpreted
	if (write_count == 0) {
		address = 0;
		// now we interpret new data
		command_id = data;
		// reset SPIDATA too
		SPIDATA = 0;
	}

	// set the busy flag
	SPICNT |= (1 << 7);
	// do transfer
	// check device select
	switch ((SPICNT >> 8) & 0x3) {
	case 0:
		SPIDATA = 0;
		break;
	case 1:
		// firmware
		SPIDATA = firmware_transfer(data);
		break;
	default:
		log_fatal("device %d in firmware write_spidata not implemented", (SPICNT >> 8) & 0x3);
	}


	// clear the busy flag
	SPICNT &= ~(1 << 7);

	// check if chip must be deselected
	if (get_bit(11, SPICNT)) {
		write_count++;
	} else {
		// reset write_count to 0 so new command can be interpreted
		write_count = 0;
	}

	if (get_bit(14, SPICNT)) {
		if (nds->interrupt[0].IME) {
			log_fatal("oops interrupts not handled yet");
		}
		
	}
}

u8 SPI::firmware_transfer(u8 command) {
	u8 output = 0;

	// rlly dont know what im doing lol
	switch (command_id) {
	case READ_STREAM:
		if (write_count < 4) {
			// set up address
			address <<= 8;
			address |= command;
			output = 0;
		} else {
			// read data from firmware
			output = firmware[address & 0x3FFFF];
			address++;
		}
		break;
	default:
		log_fatal("firmware command 0x%04x unimplemented!", command);
	}


	return output;
}

void SPI::load_firmware() {
    FILE *file_buffer = fopen("../firmware/firmware.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening firmware! make sure the file firmware.bin exists in the firmware folder\n");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(firmware, FIRMWARE_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] firmware loaded successfully!");
}