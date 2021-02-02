#include <nds/nds.h>
#include <nds/cartridge.h>

NDS_Cartridge::NDS_Cartridge(NDS *nds) : nds(nds) {

}

NDS_Cartridge::~NDS_Cartridge() {
	// delete only if not a nullptr
	if (rom) {
		delete[] rom;
	}
}

void NDS_Cartridge::load_cartridge(std::string rom_path) {
	printf("rom path is %s\n", rom_path.c_str());
    FILE *file_buffer = fopen(rom_path.c_str(), "rb");
    if (file_buffer == NULL) {
        log_fatal("[Cartridge] error while opening the selected rom! make you sure specify the path correctly");
    }
    fseek(file_buffer, 0, SEEK_END);
    int cartridge_size = ftell(file_buffer);
    fseek(file_buffer, 0, SEEK_SET);
    rom = new u8[cartridge_size];
    fread(rom, 1, cartridge_size, file_buffer);
    fclose(file_buffer);  
    printf("[Cartridge] cartridge loaded successfully!\n");
    load_header_data();
}

void NDS_Cartridge::load_header_data() {
    // load the game title
    for (int i = 0; i < 12; i++) {
        header.game_title[i] = rom[i];
    }

    // todo: change to memcpy

    // load gamecode and makercode
    header.gamecode = (rom[0xC] << 24 | rom[0xD] << 16 | rom[0xE] << 8 | rom[0xF]);
    header.makercode = (rom[0x10] << 8 | rom[0x11]);

    // load arm9 stuff
    header.arm9_rom_offset = (rom[0x23] << 24 | rom[0x22] << 16 | rom[0x21] << 8 | rom[0x20]);
    header.arm9_entry_address = (rom[0x27] << 24 | rom[0x26] << 16 | rom[0x25] << 8 | rom[0x24]);
    header.arm9_ram_address = (rom[0x2B] << 24 | rom[0x2A] << 16 | rom[0x29] << 8 | rom[0x28]);
    header.arm9_size = (rom[0x2F] << 24 | rom[0x2E] << 16 | rom[0x2D] << 8 | rom[0x2C]);

    header.arm7_rom_offset = (rom[0x33] << 24 | rom[0x32] << 16 | rom[0x31] << 8 | rom[0x30]);
    header.arm7_entry_address = (rom[0x37] << 24 | rom[0x36] << 16 | rom[0x35] << 8 | rom[0x34]);
    header.arm7_ram_address = (rom[0x3B] << 24 | rom[0x3A] << 16 | rom[0x39] << 8 | rom[0x38]);
    header.arm7_size = (rom[0x3F] << 24 | rom[0x3E] << 16 | rom[0x3D] << 8 | rom[0x3C]);
    log_debug("arm9 offset: 0x%08x arm9 entry address: 0x%08x arm9 ram address: 0x%08x arm9 size: 0x%08x", header.arm9_rom_offset, header.arm9_entry_address, header.arm9_ram_address, header.arm9_size);
    log_debug("arm7 offset: 0x%08x arm7 entry address: 0x%08x arm7 ram address: 0x%08x arm7 size: 0x%08x", header.arm7_rom_offset, header.arm7_entry_address, header.arm7_ram_address, header.arm7_size);

    log_debug("[Cartridge] header data loaded successfully!");
}

void NDS_Cartridge::transfer_rom() {
    // first transfer header data to main memory
    for (u32 i = 0; i < 0x170; i++) {
        nds->memory.arm9_write_byte(0x027FFE00 + i, rom[i]);

    }
    
    // transfer arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) { 
        nds->memory.arm9_write_byte(header.arm9_ram_address + i, rom[header.arm9_rom_offset + i]);
    }
    
    // transfer arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {
        nds->memory.arm7_write_byte(header.arm7_ram_address + i, rom[header.arm7_rom_offset + i]);
    }
    // exit(1);
}

void NDS_Cartridge::write_romctrl(u32 value) {
	bool old_block_start = ROMCTRL >> 31;

	ROMCTRL = value & 0xDF7F7FFF;

	bool execute_command = false;
	if ((ROMCTRL >> 31) && !old_block_start) {
		execute_command = true;
	}
	
	if (!execute_command) {
		return;
	}

	// now do actual command lol
	do_command();
	
}

void NDS_Cartridge::do_command() {
	// check data block size
	u32 block_size;
	switch ((ROMCTRL >> 24) & 0x7) {
	case 0:
		block_size = 0;
		break;
	case 7:
		block_size = 4;
		break;
	default:
		block_size = 0x100 << ((ROMCTRL >> 24) & 0x7);
		break;
	}

	// if (block_size == 0) {
	// 	return;
	// }

	// only execute command if block start is set and data word status is cleared (0=busy)
	if ((get_bit(31, ROMCTRL)) && (!get_bit(23, ROMCTRL))) {
		switch (command_buffer[0]) {
		case 0x00:
			// return header data:
			// however for now we will just return 0xFFFFFFFF lol
			data_output = 0xFFFFFFFF;
			// set to word ready
			ROMCTRL |= (1 << 23);
			break;
		case 0x3C:
			// key1 encryption enabled lol
			key1_encryption = true;
			break;
		case 0x90:
			// get chip id
			// we can pretty much use any value for this
			// we are using some macronix id thing :shrug:
			data_output = 0x00003FC2;
			// set to word ready
			ROMCTRL |= (1 << 23);
			break;
		case 0x9F:
			// dummy command: this puts 0xFF for how many bytes in data_output
			data_output = 0xFFFFFFFF;
			break;
		default:
			log_fatal("cartridge command 0x%02x is not implemented yet!", command_buffer[0]);
		}
	}

	// deal with end of command
	if (get_bit(14, AUXSPICNT)) {
		if (nds->interrupt[0].IME) {
			log_fatal("handle");
		}
		
	}

	// after finishing command set block start to 0 (ready for next command rlly)
	ROMCTRL &= ~(1 << 31);
}

void NDS_Cartridge::write_auxspicnt(u16 value) {
	// preserve not used bits and bit 7 as that cant be changed on writes shrug
	AUXSPICNT = (AUXSPICNT & 0x80) | (value & 0xE043);
}

void NDS_Cartridge::write_hi_auxspicnt(u16 value) {
	AUXSPICNT = (AUXSPICNT & 0xFF) | ((value << 8) & 0xE043);
}

void NDS_Cartridge::write_auxspidata(u16 value) {
	// set the busy flag in auxspicnt
	AUXSPICNT |= (1 << 7);

	// idk wtf im doing but lets put 0xFF in auxspidata
	AUXSPIDATA = 0xFF;

	// busy flag is cleared after cartridge transfer (fake for now)
	AUXSPICNT &= ~(1 << 7);
}