#include <nds/memory.h>
#include <nds/nds.h>
#include <common/log.h>

static const int ARM9_BIOS_SIZE = 32 * 1024;
static const int ARM7_BIOS_SIZE = 16 * 1024;

Memory::Memory(NDS *nds): nds(nds) {

}

void Memory::reset() {
	// initialise arrays and variables correctly
	memset(main_memory, 0, 0x400000);
	memset(shared_wram, 0, 0x8000);
	memset(arm9_bios, 0, 0x8000);
	memset(arm7_bios, 0, 0x4000);
	memset(arm7_wram, 0, 0x10000);

	POSTFLG7 = POSTFLG9 = 0;

	WRAMCNT = 0;
	EXMEMCNT = 0;
}

u8 Memory::arm7_read_byte(u32 addr) {
	switch (addr & 0xFF000000) {
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		return main_memory[addr & 0x3FFFFF];
	case 0x03000000:
		// TODO: change this to regular writes
		// ok in shared wram it is split into repeating blocks for arm7 and arm9
		switch (WRAMCNT) {
		case 0:
			// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
			return arm7_wram[addr & 0xFFFF];
		case 1:
			// the first block is for 16k of arm7 wram and next 16k block is for arm9
			return shared_wram[addr & 0x3FFF];
		case 2:
			// the first block is 16k for arm9 wram and next 16k block is for arm7
			return shared_wram[(addr & 0x3FFF) + 0x4000];
		case 3:
			// 32 k for arm7 and 0k for arm9
			return shared_wram[addr & 0x7FFF];
		}
		// as always if in the arm7 wram region just write to it normally instead of having to write when wramcnt = 0
		if (addr >= 0x03800000) {
			return arm7_wram[addr & 0xFFFF];
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 8bit reading at address 0x%08x", addr);
		#endif
		switch (addr) {
		case 0x04000138:
			return nds->rtc.control_register;
		case 0x04000300:
			return POSTFLG7;
		case 0x040001C2:
			return nds->spi.SPIDATA;
		default:
			log_fatal("unimplemented 8 bit arm7 io read at address 0x%08x", addr);
		}
		break;
	default:
		log_fatal("unimplemented 8 bit arm7 read at address 0x%08x", addr);
	}
}

u16 Memory::arm7_read_halfword(u32 addr) {
	addr &= ~1;

	u16 return_value = 0;

	switch (addr & 0xFF000000) {
	case 0x00000000:
		memcpy(&return_value, &arm7_bios[addr & 0x3FFF], 2);
		break;
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 2);
		break;
	case 0x03000000:
		if (addr >= 0x03800000) {
			memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 2);
		} else {
			switch (WRAMCNT) {
			case 0:
				// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
				memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 2);
				break;
			case 1:
				// the first block is for 16k of arm7 wram and next 16k block is for arm9
				memcpy(&return_value, &shared_wram[addr & 0x3FFF], 2);
				break;
			case 2:
				// the first block is 16k for arm9 wram and next 16k block is for arm7
				memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 2);
				break;
			case 3:
				// 32 k for arm7 and 0k for arm9
				memcpy(&return_value, &shared_wram[addr & 0x7FFF], 2);
				break;
			}
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 16bit reading at address 0x%08x", addr);
		#endif
		switch (addr) {
		case 0x04000004:
			return nds->gpu.DISPSTAT7;
		case 0x040000BA:
			return nds->dma[0].dma_channel[0].DMACNT_H;
		case 0x04000130:
			return nds->input.KEYINPUT;
		case 0x04000180:
			return nds->ipc.read_ipcsync7();
		case 0x04000184:
			return nds->ipc.IPCFIFOCNT7;
		case 0x040001C0:
			return nds->spi.SPICNT;
		case 0x040001C2:
			return nds->spi.SPIDATA;
		default:
			log_fatal("unimplemented 16 bit arm7 io read at address 0x%08x", addr);
		}

		break;
	default:
		log_fatal("unimplemented 16 bit arm7 read at address 0x%08x", addr);
	}

	return return_value;
}

u32 Memory::arm7_read_word(u32 addr) {
	addr &= ~3;

	u32 return_value = 0;

	switch (addr & 0xFF000000) {
	case 0x00000000:
		memcpy(&return_value, &arm7_bios[addr & 0x3FFF], 4);
		break;
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 4);
		break;
	case 0x03000000:
		if (addr >= 0x03800000) {
			memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 4);
		} else {
			switch (WRAMCNT) {
			case 0:
				// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
				memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 4);
				break;
			case 1:
				// the first block is for 16k of arm7 wram and next 16k block is for arm9
				memcpy(&return_value, &shared_wram[addr & 0x3FFF], 4);
				break;
			case 2:
				// the first block is 16k for arm9 wram and next 16k block is for arm7
				memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 4);
				break;
			case 3:
				// 32 k for arm7 and 0k for arm9
				memcpy(&return_value, &shared_wram[addr & 0x7FFF], 4);
				break;
			}
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 reading from address 0x%08x", addr);
		#endif
		switch (addr) {
		case 0x04000180:
			return nds->ipc.read_ipcsync7();
		case 0x040001A4:
			return nds->cartridge.ROMCTRL;
		case 0x040001C0:
			return ((nds->spi.SPIDATA << 16) | nds->spi.SPICNT);
		case 0x04000208:
			return nds->interrupt[0].IME;
		case 0x04000210:
			return nds->interrupt[0].IE;
		case 0x04100010:
			return nds->cartridge.data_output;
		default:
			log_fatal("unimplemented 32 bit arm7 io read at address 0x%08x", addr);
		}
	default:
		log_fatal("unimplemented 32 bit arm7 read at address 0x%08x", addr);
	}

	return return_value;
}

void Memory::arm7_write_byte(u32 addr, u8 data) {
	// ignore arm7 bios writes
	if (addr < 0x4000) {
		return;
	}

	switch (addr & 0xFF000000) {
	case 0x02000000:
		main_memory[addr & 0x3FFFFF] = data;
		break;
	case 0x03000000:
		
		// as always if in the arm7 wram region just write to it normally instead of having to write when wramcnt = 0
		if (addr >= 0x03800000) {
			arm7_wram[addr & 0xFFFF] = data;
		} else {
			// ok in shared wram it is split into repeating blocks for arm7 and arm9
			switch (WRAMCNT) {
			case 0:
				// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
				arm7_wram[addr & 0xFFFF] = data;
				break;
			case 1:
				// the first block is for 16k of arm7 wram and next 16k block is for arm9
				shared_wram[addr & 0x3FFF] = data;
				break;
			case 2:
				// the first block is 16k for arm9 wram and next 16k block is for arm7
				shared_wram[(addr & 0x3FFF) + 0x4000] = data;
				break;
			case 3:
				// 32 k for arm7 and 0k for arm9
				shared_wram[addr & 0x7FFF] = data;
				break;
			}
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 writing data 0x%02x to address 0x%08x", data, addr);
		#endif
		switch (addr) {
		case 0x04000138:
			nds->rtc.control_register = data;
			return;
		case 0x040001A1:
			// set the high byte of auxspicnt
			nds->cartridge.write_hi_auxspicnt(data);
			return;

		// could probably reduce loc here
		case 0x040001A8:
			nds->cartridge.command_buffer[0] = data;
			return;
		case 0x040001A9:
			nds->cartridge.command_buffer[1] = data;
			return;
		case 0x040001AA:
			nds->cartridge.command_buffer[2] = data;
			return;
		case 0x040001AB:
			nds->cartridge.command_buffer[3] = data;
			return;
		case 0x040001AC:
			nds->cartridge.command_buffer[4] = data;
			return;
		case 0x040001AD:
			nds->cartridge.command_buffer[5] = data;
			return;
		case 0x040001AE:
			nds->cartridge.command_buffer[6] = data;
			return;
		case 0x040001AF:
			nds->cartridge.command_buffer[7] = data;
			return;
		case 0x040001C2:
			nds->spi.write_spidata(data);
			return;
		case 0x04000208:
			nds->interrupt[0].write_ime(data);
			return;
		case 0x04000301:
			switch (data) {
			// only allow halting
			case 0x80:
				nds->arm7.halt();
				break;
			default:
				log_fatal("unknown HALTCNT state 0x%02x!", data);
			}
			return;
		default:
			log_fatal("unimplemented 8 bit arm7 io write at address 0x%08x with data 0x%02x", addr, data);
		}
		break;
	default:
		log_fatal("unimplemented 8 bit arm7 write at address 0x%08x with data 0x%02x", addr, data);
	}
}

void Memory::arm7_write_halfword(u32 addr, u16 data) {
	addr &= ~1;

	switch (addr & 0xFF000000) {
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		memcpy(&main_memory[addr & 0x3FFFFF], &data, 2);
		break;
	case 0x03000000:
		// as always if in the arm7 wram region just write to it normally instead of having to write when wramcnt = 0
		if (addr >= 0x03800000) {
			memcpy(&arm7_wram[addr & 0xFFFF], &data, 2);
		} else {
			// ok in shared wram it is split into repeating blocks for arm7 and arm9
			switch (WRAMCNT) {
			case 0:
				// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
				memcpy(&arm7_wram[addr & 0xFFFF], &data, 2);
				break;
			case 1:
				// the first block is for 16k of arm7 wram and next 16k block is for arm9
				memcpy(&shared_wram[addr & 0x3FFF], &data, 2);
				break;
			case 2:
				// the first block is 16k for arm9 wram and next 16k block is for arm7
				memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, 2);
				break;
			case 3:
				// 32 k for arm7 and 0k for arm9
				memcpy(&shared_wram[addr & 0x7FFF], &data, 2);
				break;
			}
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 writing data 0x%08x to address 0x%04x", data, addr);
		#endif
		switch (addr) {
		case 0x040000BA:
			nds->dma[0].write_dmacnt_h(0, data);
			break;
		case 0x0400010C:
			nds->timers[0].write_tmcnt_l(3, data);
			break;
		case 0x0400010E:
			printf("jew\n");
			nds->timers[0].write_tmcnt_h(3, data);
			break;
		case 0x04000180:
			nds->ipc.write_ipcsync7(data);
			break;
		case 0x04000184:
			nds->ipc.write_ipcfifocnt7(data);
			break;
		case 0x040001C0:
			nds->spi.write_spicnt(data);
			break;
		case 0x040001C2:
			nds->spi.write_spidata(data);
			break;
		case 0x04000208:
			nds->interrupt[0].write_ime(data);
			return;
		default:
			log_fatal("unimplemented 16 bit arm7 io write at address 0x%08x with data 0x%04x", addr, data);
		}

		break;
	default:
		log_fatal("unimplemented 16 bit arm7 write at address 0x%08x with data 0x%04x", addr, data);
	}
}
void Memory::arm7_write_word(u32 addr, u32 data) {
	addr &= ~3;

	switch (addr & 0xFF000000) {
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		memcpy(&main_memory[addr & 0x3FFFFF], &data, 4);
		break;
	case 0x03000000:
		// as always if in the arm7 wram region just write to it normally instead of having to write when wramcnt = 0
		if (addr >= 0x03800000) {
			memcpy(&arm7_wram[addr & 0xFFFF], &data, 4);
		} else {
			// ok in shared wram it is split into repeating blocks for arm7 and arm9
			switch (WRAMCNT) {
			case 0:
				// 32k for arm9 and 0k for arm7 so we then write to arm7 wram (0x03800000 and up)
				memcpy(&arm7_wram[addr & 0xFFFF], &data, 4);
				break;
			case 1:
				// the first block is for 16k of arm7 wram and next 16k block is for arm9
				memcpy(&shared_wram[addr & 0x3FFF], &data, 4);
				break;
			case 2:
				// the first block is 16k for arm9 wram and next 16k block is for arm7
				memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, 4);
				break;
			case 3:
				// 32 k for arm7 and 0k for arm9
				memcpy(&shared_wram[addr & 0x7FFF], &data, 4);
				break;
			}
		}
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm7 writing data 0x%08x to address 0x%08x", data, addr);
		#endif
		switch (addr) {
		// case 0x04000000:

		case 0x04000100:
			nds->timers[0].write_tmcnt_l(0, data & 0xFFFF);
			nds->timers[0].write_tmcnt_h(0, data >> 16);
			break;
		case 0x04000104:
			nds->timers[0].write_tmcnt_l(1, data & 0xFFFF);
			nds->timers[0].write_tmcnt_h(1, data >> 16);
			break;
		case 0x04000180:
			nds->ipc.write_ipcsync7(data);
			break;
		case 0x040001A4:
			nds->cartridge.write_romctrl(data);
			break;
		case 0x04000208:
			nds->interrupt[0].write_ime(data);
			break;
		case 0x04000210:
			nds->interrupt[0].write_ie7(data);
			break;
		case 0x04000214:
			nds->interrupt[0].write_if(data);
			break;
		default:
			log_fatal("unimplemented 32 bit arm7 io write at address 0x%08x with data 0x%08x", addr, data);
		}

		break;
	default:
		log_fatal("unimplemented 32 bit arm7 write at address 0x%08x with data 0x%08x", addr, data);
	}
}

u8 Memory::arm9_read_byte(u32 addr) {
	switch (addr & 0xFF000000) {
	case 0x02000000:
		return main_memory[addr & 0x3FFFFF];
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm9 reading data from address 0x%08x", addr);
		#endif
		switch (addr) {
		case 0x04000208:
			return nds->interrupt[1].IME;
		case 0x04000300:
			return POSTFLG9;
		case 0x04004000:
			return 0;
		default:
			log_fatal("unimplemented 8 bit arm9 io read at address 0x%08x", addr);
		}
	
	default:
		log_fatal("unimplemented 8 bit arm9 read at address 0x%08x", addr);
	}
}

u16 Memory::arm9_read_halfword(u32 addr) {
	addr &= ~1;

	u16 return_value = 0;

	if (((nds->cp15.get_itcm_enabled()) && (addr < nds->cp15.get_itcm_size()))) {
		memcpy(&return_value, &nds->cp15.itcm[addr & 0x7FFF], 2);
	} else if ((nds->cp15.get_dtcm_enabled()) && ((addr >= nds->cp15.get_dtcm_base_addr()) && (addr < (nds->cp15.get_dtcm_base_addr() + nds->cp15.get_dtcm_size())))) {
		memcpy(&return_value, &nds->cp15.dtcm[(addr - nds->cp15.get_dtcm_base_addr()) & 0x3FFF], 2);
	} else {
		switch (addr & 0xFF000000) {
		case 0x02000000:
			// for now we will ignore main memory control because seems to only matter for power down????
			memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 2);
			break;
		case 0x03000000:
			switch (WRAMCNT) {
			case 0:
				// 32 kb for arm9
				memcpy(&return_value, &shared_wram[addr & 0x7FFF], 2);
				break;
			case 1:
				// first block is 16kb for arm7 and second block is 16kb for arm9
				memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 2);
				break;
			case 2:
				// first block is 16kb for arm9 and second block is 16kb for arm7
				memcpy(&return_value, &shared_wram[addr & 0x3FFF], 2);
				break;
			case 3:
				// 0kb for arm9 which means share wram is empty so just return 0
				return 0;
			}
			break;
		case 0x04000000:
			#ifdef IO_DEBUG
			log_warn("arm9 reading data from address 0x%08x", addr);
			#endif
			switch (addr) {
			case 0x04000004:
				return nds->gpu.DISPSTAT9;
			case 0x04000130:
				return nds->input.KEYINPUT;
			case 0x04000180:
				return nds->ipc.read_ipcsync9();
			default:
				log_fatal("unimplemented 16 bit arm9 io read at address 0x%08x", addr);
			}

			break;
		case 0xFF000000:
			memcpy(&return_value, &arm9_bios[addr & 0x7FFF], 2);
			break;
		default:
			log_fatal("unimplemented 16 bit arm9 read at address 0x%08x", addr);
		}
	}

	return return_value;
}

u32 Memory::arm9_read_word(u32 addr) {
	addr &= ~3;

	u32 return_value = 0;
	if (((nds->cp15.get_itcm_enabled()) && (addr < nds->cp15.get_itcm_size()))) {
		memcpy(&return_value, &nds->cp15.itcm[addr & 0x7FFF], 4);
	} else if ((nds->cp15.get_dtcm_enabled()) && ((addr >= nds->cp15.get_dtcm_base_addr()) && (addr < (nds->cp15.get_dtcm_base_addr() + nds->cp15.get_dtcm_size())))) {
		memcpy(&return_value, &nds->cp15.dtcm[(addr - nds->cp15.get_dtcm_base_addr()) & 0x3FFF], 4);
	} else {
		switch (addr & 0xFF000000) {
		case 0x02000000:
			memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 4);
			break;
		case 0x03000000:
			switch (WRAMCNT) {
			case 0:
				// 32 kb for arm9
				memcpy(&return_value, &shared_wram[addr & 0x7FFF], 4);
				break;
			case 1:
				// first block is 16kb for arm7 and second block is 16kb for arm9
				memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 4);
				break;
			case 2:
				// first block is 16kb for arm9 and second block is 16kb for arm7
				memcpy(&return_value, &shared_wram[addr & 0x3FFF], 4);
				break;
			case 3:
				// 0kb for arm9 which means share wram is empty so just return 0
				return 0;
			}
			break;
		case 0x04000000:
			#ifdef IO_DEBUG
			log_warn("arm9 reading data from address 0x%08x", addr);
			#endif
			switch (addr) {
			case 0x040000DC:
				// read dma3cnt_l and dma3cnt_h
				return ((((nds->dma[1].dma_channel[3].DMACNT_H) & 0xFFFF) << 16) | ((nds->dma[1].dma_channel[3].DMACNT_L) & 0xFFFF));
			case 0x040000EC:
				return DMAFILL[3];
			case 0x04000180:
				return nds->ipc.read_ipcsync9();
			case 0x04000208:
				return nds->interrupt[1].IME;
			case 0x04000210:
				return nds->interrupt[1].IE;
			case 0x04000214:
				return nds->interrupt[1].IF;
			case 0x04000240:
				return (nds->gpu.vramcnt_d << 24 | nds->gpu.vramcnt_c << 16 | nds->gpu.vramcnt_b << 8 | nds->gpu.vramcnt_a);
			case 0x04004000:
            	return 0;
			case 0x04004008:
				return 0;
			default:
				log_fatal("unimplemented 32 bit arm9 io read at address 0x%08x", addr);
			}
			break;
		case 0xFF000000:
			memcpy(&return_value, &arm9_bios[addr & 0x7FFF], 4);
			break;
		default:
			log_fatal("unimplemented 32 bit arm9 read at address 0x%08x", addr);
		}
	}


		

	return return_value;
}

void Memory::arm9_write_byte(u32 addr, u8 data) {
	switch (addr & 0xFF000000) {
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		main_memory[addr & 0x3FFFFF] = data;
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm9 writing data 0x%08x to address 0x%04x", data, addr);
		#endif
		switch (addr) {
		case 0x04000208:
			nds->interrupt[1].write_ime(data);
			return;
		case 0x04000240:
			nds->gpu.vramcnt_a = data;
			return;
		case 0x04000241:
			nds->gpu.vramcnt_b = data;
			return;
		case 0x04000242:
			nds->gpu.vramcnt_c = data;
			return;
		case 0x04000243:
			nds->gpu.vramcnt_d = data;
			return;
		case 0x04000244:
			nds->gpu.vramcnt_e = data;
			return;
		case 0x04000245:
			nds->gpu.vramcnt_f = data;
			return;
		case 0x04000246:
			nds->gpu.vramcnt_g = data;
			return;
		case 0x04000247:
			// only bits 0..1 are used
			WRAMCNT = data & 0x3;
			return;
		case 0x04000248:
			nds->gpu.vramcnt_h = data;
			return;
		case 0x04000249:
			nds->gpu.vramcnt_i = data;
			return;
		default:
			log_fatal("unimplemented 8 bit arm9 io write at address 0x%08x with data 0x%02x", addr, data);
		}

		break;
	default:
		log_fatal("unimplemented 8 bit arm9 write at address 0x%08x with data 0x%02x", addr, data);
	}
}

void Memory::arm9_write_halfword(u32 addr, u16 data) {
	addr &= ~1;

	switch (addr & 0xFF000000) {
	case 0x02000000:
		// for now we will ignore main memory control because seems to only matter for power down????
		memcpy(&main_memory[addr & 0x3FFFFF], &data, 2);
		break;
	case 0x04000000:
		#ifdef IO_DEBUG
		log_warn("arm9 writing data 0x%08x to address 0x%04x", data, addr);
		#endif
		switch (addr) {
		case 0x04000004:
			nds->gpu.write_dispstat9(data);
			break;
		case 0x040000B8:
			nds->dma[1].write_dmacnt_l(0, data);
			break;
		case 0x040000BA:
			nds->dma[1].write_dmacnt_h(0, data);
			break;
		case 0x040000D0:
			nds->dma[1].write_dmacnt_l(2, data);
			break;
		case 0x04000100:
			// write to TM0CNT_L for arm9
			nds->timers[1].write_tmcnt_l(0, data);
			break;
		case 0x04000102:
			// write to TM0CNT_H for arm9
			nds->timers[1].write_tmcnt_h(0, data);
			break;
		case 0x04000104:
			// write to TM1CNT_L for arm9
			nds->timers[1].write_tmcnt_l(1, data);
			break;
		case 0x04000106:
			// write to TM1CNT_H for arm9
			nds->timers[1].write_tmcnt_h(1, data);
			break;
		case 0x04000108:
			// write to TM2CNT_L for arm9
			nds->timers[1].write_tmcnt_l(2, data);
			break;
		case 0x0400010A:
			// write to TM2CNT_H for arm9
			nds->timers[1].write_tmcnt_h(2, data);
			break;
		case 0x0400010C:
			// write to TM3CNT_L for arm9
			nds->timers[1].write_tmcnt_l(3, data);
			break;
		case 0x0400010E:
			// write to TM3CNT_H for arm9
			nds->timers[1].write_tmcnt_h(3, data);
			break;
		case 0x04000180:
			nds->ipc.write_ipcsync9(data);
			break;
		case 0x04000184:
			nds->ipc.write_ipcfifocnt9(data);
			break;
		case 0x04000204:
			// TODO: handle masking bits later
			EXMEMCNT = data;
			break;
		case 0x04000208:
			nds->interrupt[1].write_ime(data);
			break;
		case 0x04000304:
			nds->gpu.POWCNT1 = data;
			break;
		default:
			log_fatal("unimplemented 16 bit arm9 io write at address 0x%08x with data 0x%04x", addr, data);
		}

		break;
	case 0x06000000:
		if (addr >= 0x06800000) {
			// write to lcdc vram
			nds->gpu.write_lcdc_vram(addr, data);
		} else {
			log_fatal("unimplemented 16 bit arm9 write at address 0x%08x with data 0x%04x", addr, data);
		}
		break;
	default:
		log_warn("unimplemented 16 bit arm9 write at address 0x%08x with data 0x%04x", addr, data);
		return;
	}
}
void Memory::arm9_write_word(u32 addr, u32 data) {
	addr &= ~3;
	
	// check itcm and dtcm first
	// if itcm and dtcm have overlap itcm seems to have priority
	if (((nds->cp15.get_itcm_enabled()) && (addr < nds->cp15.get_itcm_size()))) {
		memcpy(&nds->cp15.itcm[addr & 0x7FFF], &data, 4);
		// done with the write
		return;
	} else if ((nds->cp15.get_dtcm_enabled()) && ((addr >= nds->cp15.get_dtcm_base_addr()) && (addr < (nds->cp15.get_dtcm_base_addr() + nds->cp15.get_dtcm_size())))) {
		memcpy(&nds->cp15.dtcm[(addr - nds->cp15.get_dtcm_base_addr()) & 0x3FFF], &data, 4);
		
		// done with the write
		return;
	} else {
		switch (addr & 0xFF000000) {
		case 0x02000000:
			// for now we will ignore main memory control because seems to only matter for power down????
			memcpy(&main_memory[addr & 0x3FFFFF], &data, 4);
			break;
		case 0x03000000:
			switch (WRAMCNT) {
			case 0:
				// 32 kb for arm9
				memcpy(&shared_wram[addr & 0x7FFF], &data, 4);
				break;
			case 1:
				// first block is 16kb for arm7 and second block is 16kb for arm9
				memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, 4);
				break;
			case 2:
				// first block is 16kb for arm9 and second block is 16kb for arm7
				memcpy(&shared_wram[addr & 0x3FFF], &data, 4);
				break;
			case 3:
				// 0kb for arm9 which means share wram is empty so just return 0
				// do nothing lol
				break;
			}
			break;
		case 0x04000000:
			#ifdef IO_DEBUG
			log_warn("arm9 writing data 0x%08x to address 0x%08x", data, addr);
			#endif
			switch (addr) {
			case 0x04000000:
				// write to DISPCNT for engine
				nds->gpu.engine_a.DISPCNT = data;
				break;
			case 0x04000004:
				// write to DISPSTAT
				nds->gpu.write_dispstat9(data & 0xFFFF);
				break;
			case 0x04000008:
				// write to BG0CNT and BG1CNT (engine a)
				nds->gpu.engine_a.BGCNT[0] = data & 0xFFFF;
				nds->gpu.engine_a.BGCNT[1] = data >> 16;
				break;
			case 0x0400000C:
				// write to BG2CNT and BG3CNT (engine a)
				nds->gpu.engine_a.BGCNT[2] = data & 0xFFFF;
				nds->gpu.engine_a.BGCNT[3] = data >> 16;
				break;
			case 0x04000010:
				// write to BG0HOFS and BG1HOFS (engine a)
				nds->gpu.engine_a.BGHOFS[0] = data & 0xFFFF;
				nds->gpu.engine_a.BGHOFS[1] = data >> 16;
				break;
			case 0x04000014:
				// write to BG2HOFS and BG3HOFS (engine a)
				nds->gpu.engine_a.BGHOFS[2] = data & 0xFFFF;
				nds->gpu.engine_a.BGHOFS[3] = data >> 16;
				break;
			case 0x04000018:
				// write to BG0VOFS and BG1VOFS (engine a)
				nds->gpu.engine_a.BGVOFS[0] = data & 0xFFFF;
				nds->gpu.engine_a.BGVOFS[1] = data >> 16;
				break;
			case 0x0400001C:
				// write to BG2VOFS and BG3VOFS (engine a)
				nds->gpu.engine_a.BGVOFS[2] = data & 0xFFFF;
				nds->gpu.engine_a.BGVOFS[3] = data >> 16;
				break;
			case 0x04000020:
				// write to BG2PA and BG2PB (engine a)
				nds->gpu.engine_a.BG2PA = data & 0xFFFF;
				nds->gpu.engine_a.BG2PB = data >> 16;
				break;
			case 0x04000024:
				// write to BG2PC and BG2PD (engine a)
				nds->gpu.engine_a.BG2PC = data & 0xFFFF;
				nds->gpu.engine_a.BG2PD = data >> 16;
				break;
			case 0x04000028:
				// write to BG2X (engine a)
				nds->gpu.engine_a.BG2X = data;
				break;
			case 0x0400002C:
				// write to BG2Y (engine a)
				nds->gpu.engine_a.BG2Y = data;
				break;
			case 0x04000030:
				// write to BG3PA and BG3PB
				nds->gpu.engine_a.BG3PA = data & 0xFFFF;
				nds->gpu.engine_a.BG3PB = data >> 16;
				break;
			case 0x04000034:
				// write to BG3PC and BG3PD
				nds->gpu.engine_a.BG3PC = data & 0xFFFF;
				nds->gpu.engine_a.BG3PD = data >> 16;
				break;
			case 0x04000038:
				// write to BG3X
				nds->gpu.engine_a.BG3X = data;
				break;
			case 0x0400003C:
				// write to BG3Y
				nds->gpu.engine_a.BG3Y = data;
				break;
			case 0x04000040:
				// write to WIN0H and WIN1H
				nds->gpu.engine_a.WIN0H = data & 0xFFFF;
				nds->gpu.engine_a.WIN1H = data >> 16;
				break;
			case 0x04000044:
				// write to WIN0V and WIN1V
				nds->gpu.engine_a.WIN0V = data & 0xFFFF;
				nds->gpu.engine_a.WIN1V = data >> 16;
				break;
			case 0x04000048:
				// write to WININ and WINOUT
				nds->gpu.engine_a.WININ = data & 0xFFFF;
				nds->gpu.engine_a.WINOUT = data >> 16;
				break;
			case 0x0400004C:
				// write to MOSAIC
				nds->gpu.engine_a.MOSAIC = data & 0xFFFF;
				break;
			case 0x04000050:
				// write to BLDCNT and BLDALPHA
				nds->gpu.engine_a.BLDCNT = data & 0xFFFF;
				nds->gpu.engine_a.BLDALPHA = data >> 16;
				break;
			case 0x040000B0:
				// write to DMA0SAD
				nds->dma[1].dma_channel[0].DMASAD = data;
				break;
			case 0x040000B4:
				// write to DMA0DAD
				nds->dma[1].dma_channel[0].DMADAD = data;
				break;
			case 0x040000B8:
				// write to DMA0CNT_L and DMA0CNT_H
				nds->dma[1].write_dmacnt_l(0, data & 0xFFFF);
				nds->dma[1].write_dmacnt_h(0, data >> 16);
				break;
			case 0x040000BC:
				// write to DMA1SAD
				nds->dma[1].dma_channel[1].DMASAD = data;
				break;
			case 0x040000C0:
				// write to DMA1DAD
				nds->dma[1].dma_channel[1].DMADAD = data;
				break;
			case 0x040000C4:
				// write to DMA1CNT_L and DMA1CNT_H
				nds->dma[1].write_dmacnt_l(1, data & 0xFFFF);
				nds->dma[1].write_dmacnt_h(1, data >> 16);
				break;
			case 0x040000C8:
				// write to DMA2SAD
				nds->dma[1].dma_channel[2].DMASAD = data;
				break;
			case 0x040000CC:
				// write to DMA2DAD
				nds->dma[1].dma_channel[2].DMADAD = data;
				break;
			case 0x040000D0:
				// write to DMA2CNT_L and DMA2CNT_H
				nds->dma[1].write_dmacnt_l(2, data & 0xFFFF);
				nds->dma[1].write_dmacnt_h(2, data >> 16);
				break;
			case 0x040000D4:
				// write to DMA3SAD
				nds->dma[1].dma_channel[3].DMASAD = data;
				break;
			case 0x040000D8:
				// write to DMA3DAD
				nds->dma[1].dma_channel[3].DMADAD = data;
				break;
			case 0x040000DC:
				// write to DMA3CNT_L and DMA3CNT_H
				nds->dma[1].write_dmacnt_l(3, data & 0xFFFF);
				nds->dma[1].write_dmacnt_h(3, data >> 16);
				break;
			case 0x040000EC:
				// write to DMA3FILL
				DMAFILL[3] = data;
				break;
			case 0x04000180:
				nds->ipc.write_ipcsync9(data);
				break;
			case 0x040001A0:
				// write to AUXSPICNT
				nds->cartridge.write_auxspicnt(data & 0xFFFF);
				// write to AUXSPIDATA
				nds->cartridge.write_auxspidata((data >> 16) & 0xFF);
				break;
			case 0x040001A4:
				nds->cartridge.write_romctrl(data);
				break;
			case 0x04000208:
				nds->interrupt[1].write_ime(data);
				break;
			case 0x04000210:
				nds->interrupt[1].write_ie9(data);
				break;
			case 0x04000214:
				nds->interrupt[1].write_if(data);
				break;
			case 0x04000240:
				// sets vramcnt_a, vramcnt_b, vramcnt_c and vramcnt_d
				nds->gpu.vramcnt_a = data & 0xFF;
				nds->gpu.vramcnt_b = (data >> 8) & 0xFF;
				nds->gpu.vramcnt_c = (data >> 16) & 0xFF;
				nds->gpu.vramcnt_d = (data >> 24) & 0xFF;
				break;
			case 0x04000304:
				nds->gpu.POWCNT1 = data;
				break;
			case 0x04001000:
				// write to DISPCNT (engine b)
				nds->gpu.engine_b.DISPCNT = data;
				break;
			case 0x04001008:
				// write to BG0CNT and BG1CNT (engine b)
				nds->gpu.engine_b.BGCNT[0] = data & 0xFFFF;
				nds->gpu.engine_b.BGCNT[1] = data >> 16;
				break;
			case 0x0400100C:
				// write to BG2CNT and BG3CNT (engine b)
				nds->gpu.engine_b.BGCNT[2] = data & 0xFFFF;
				nds->gpu.engine_b.BGCNT[3] = data >> 16;
				break;
			case 0x04001010:
				// write to BG0HOFS and BG1HOFS (engine b)
				nds->gpu.engine_b.BGHOFS[0] = data & 0xFFFF;
				nds->gpu.engine_b.BGHOFS[1] = data >> 16;
				break;
			case 0x04001014:
				// write to BG2HOFS and BG3HOFS (engine b)
				nds->gpu.engine_b.BGHOFS[2] = data & 0xFFFF;
				nds->gpu.engine_b.BGHOFS[3] = data >> 16;
				break;
			case 0x04001018:
				// write to BG0VOFS and BG1VOFS (engine b)
				nds->gpu.engine_b.BGVOFS[0] = data & 0xFFFF;
				nds->gpu.engine_b.BGVOFS[1] = data >> 16;
				break;
			case 0x0400101C:
				// write to BG2VOFS and BG3VOFS (engine b)
				nds->gpu.engine_b.BGVOFS[2] = data & 0xFFFF;
				nds->gpu.engine_b.BGVOFS[3] = data >> 16;
				break;
			case 0x04001020:
				// write to BG2PA and BG2PB (engine b)
				nds->gpu.engine_b.BG2PA = data & 0xFFFF;
				nds->gpu.engine_b.BG2PB = data >> 16;
				break;
			case 0x04001024:
				// write to BG2PC and BG2PD (engine b)
				nds->gpu.engine_b.BG2PC = data & 0xFFFF;
				nds->gpu.engine_b.BG2PD = data >> 16;
				break;
			case 0x04001028:
				// write to BG2X (engine b)
				nds->gpu.engine_b.BG2X = data;
				break;
			case 0x0400102C:
				// write to BG2Y (engine b)
				nds->gpu.engine_b.BG2Y = data;
				break;
			case 0x04001030:
				// write to BG3PA and BG3PB (engine b)
				nds->gpu.engine_b.BG3PA = data & 0xFFFF;
				nds->gpu.engine_b.BG3PB = data >> 16;
				break;
			case 0x04001034:
				// write to BG3PC and BG3PD (engine b)
				nds->gpu.engine_b.BG3PC = data & 0xFFFF;
				nds->gpu.engine_b.BG3PD = data >> 16;
				break;
			case 0x04001038:
				// write to BG3X (engine b)
				nds->gpu.engine_b.BG3X = data;
				break;
			case 0x0400103C:
				// write to BG3Y (engine b)
				nds->gpu.engine_b.BG3Y = data;
				break;
			case 0x04001040:
				// write to WIN0H and WIN1H (engine b)
				nds->gpu.engine_b.WIN0H = data & 0xFFFF;
				nds->gpu.engine_b.WIN1H = data >> 16;
				break;
			case 0x04001044:
				// write to WIN0V and WIN1V (engine b)
				nds->gpu.engine_b.WIN0V = data & 0xFFFF;
				nds->gpu.engine_b.WIN1V = data >> 16;
				break;
			case 0x04001048:
				// write to WININ and WINOUT (engine b)
				nds->gpu.engine_b.WININ = data & 0xFFFF;
				nds->gpu.engine_b.WINOUT = data >> 16;
				break;
			case 0x0400104C:
				// write to MOSAIC (engine b)
				nds->gpu.engine_b.MOSAIC = data & 0xFFFF;
				break;
			case 0x04001050:
				// write to BLDCNT and BLDALPHA (engine b)
				nds->gpu.engine_b.BLDCNT = data & 0xFFFF;
				nds->gpu.engine_b.BLDALPHA = data >> 16;
				break;
			case 0x04001054:
				// write to BLDY (engine b)
				nds->gpu.engine_b.BLDY = data & 0xFFFF;
				break;
			case 0x04001058:
				// dont do anything is this address is unused (shrug)
				return;
			default:
				log_fatal("unimplemented 32 bit arm9 io write at address 0x%08x with data 0x%08x", addr, data);
			}

			break;
		case 0x05000000:
			// write to palette ram
			// check depending on the address which engines palette ram to write to
			if ((addr & 0x3FF) < 400) {
				// this is the first block which is assigned to engine a
				nds->gpu.engine_a.write_palette_ram(addr, data & 0xFFFF);
				nds->gpu.engine_a.write_palette_ram(addr + 2, data >> 16);
			} else {
				// write to engine b's palette ram
				nds->gpu.engine_b.write_palette_ram(addr, data & 0xFFFF);
				nds->gpu.engine_b.write_palette_ram(addr + 2, data >> 16);
			}
			break;
		case 0x06000000:
			if (addr >= 0x06800000) {
				nds->gpu.write_lcdc_vram(addr, data & 0xFFFF);
				nds->gpu.write_lcdc_vram(addr + 2, data >> 16);
			} else {
				log_fatal("unimplemented 32 bit arm9 write at address 0x%08x with data 0x%08x", addr, data);
			}
			break;
		case 0x07000000:
			// check memory address to see which engine to write to oam
			if ((addr & 0x3FF) < 0x400) {
				// this is the first block of oam which is 1kb and is assigned to engine a
				nds->gpu.engine_a.write_oam(addr, data & 0xFFFF);
				nds->gpu.engine_a.write_oam(addr, data >> 16);
			} else {
				// write to engine b's palette ram
				nds->gpu.engine_b.write_oam(addr, data & 0xFFFF);
				nds->gpu.engine_b.write_oam(addr + 2, data >> 16);
			}
			break;
		default:
			log_fatal("unimplemented 32 bit arm9 write at address 0x%08x with data 0x%08x", addr, data);
			return;
		}
	}
}

void Memory::load_arm9_bios() {
    FILE *file_buffer = fopen("../bios/bios9.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening arm9 bios! make sure the file bios9.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm9_bios, ARM9_BIOS_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] arm9 bios loaded successfully!");
}

void Memory::load_arm7_bios() {
    FILE *file_buffer = fopen("../bios/bios7.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("[Memory] error when opening arm7 bios! make sure the file bios7.bin exists in the bios folder\n");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm7_bios, ARM7_BIOS_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("[Memory] arm7 bios loaded successfully!");
}