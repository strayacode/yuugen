#include <nds/dma.h>
#include <nds/nds.h>

DMA::DMA(NDS *nds, bool cpu_id) : nds(nds), cpu_id(cpu_id) {

}

void DMA::transfer() {
	// check each dma channel if they are enabled, starting with channel 0 first as that has the highest priority
	for (int i = 0; i < 4; i++) {
		// only do transfer if enabled obviously
		if (enabled & (1 << i)) {
			// make variables to make things easier 
			u8 destination_address_control = (dma_channel[i].DMACNT_H >> 5) & 0x3;
			u8 source_address_control = (dma_channel[i].DMACNT_H >> 7) & 0x3;
			if (get_bit(9, dma_channel[i].DMACNT_H) || get_bit(14, dma_channel[i].DMACNT_H)) {
				log_fatal("repeat bit or irq request bit not implemented yet!");
			}

			// loop through all the data units specified by internal word count
			for (int j = 0; j < dma_channel[i].internal_word_count; j++) {
				// check if we are transferring words or halfwords
				if (get_bit(10, dma_channel[i].DMACNT_H)) {
					// transfer words
					if (cpu_id) {
						// do arm9 word transfer
						nds->memory.arm9_write_word(dma_channel[i].internal_destination_address, nds->memory.arm9_read_word(dma_channel[i].internal_source_address));
					} else {
						// do arm9 word transfer
						nds->memory.arm7_write_word(dma_channel[i].internal_destination_address, nds->memory.arm7_read_word(dma_channel[i].internal_source_address));
					}

					// we dont need case 2 as fixed doesnt do anything
					switch (destination_address_control) {
					case 0:
						dma_channel[i].internal_destination_address += 4; // increment
						break;
					case 1:
						dma_channel[i].internal_destination_address -= 4; // decrement
						break;
					case 3:
						log_fatal("increment/reload in dest addr control unimplemented!");
					}

					// we dont need case 2 as fixed doesnt do anything
					switch (source_address_control) {
					case 0:
						dma_channel[i].internal_source_address += 4; // increment
						break;
					case 1:
						dma_channel[i].internal_source_address -= 4; // decrement
						break;
					case 3:
						log_fatal("prohibited in src dest addr not allowed!");
					}
				} else {
					// transfer halfwords
					if (cpu_id) {
						// do arm9 word transfer
						nds->memory.arm9_write_halfword(dma_channel[i].internal_destination_address, nds->memory.arm9_read_halfword(dma_channel[i].internal_source_address));
					} else {
						// do arm9 word transfer
						nds->memory.arm7_write_halfword(dma_channel[i].internal_destination_address, nds->memory.arm7_read_halfword(dma_channel[i].internal_source_address));
					}

					// we dont need case 2 as fixed doesnt do anything
					switch (destination_address_control) {
					case 0:
						dma_channel[i].internal_destination_address += 2; // increment
						break;
					case 1:
						dma_channel[i].internal_destination_address -= 2; // decrement
						break;
					case 3:
						log_fatal("increment/reload in dest addr control unimplemented!");
					}

					// we dont need case 2 as fixed doesnt do anything
					switch (source_address_control) {
					case 0:
						dma_channel[i].internal_source_address += 2; // increment
						break;
					case 1:
						dma_channel[i].internal_source_address -= 2; // decrement
						break;
					case 3:
						log_fatal("prohibited in src dest addr not allowed!");
					}
				}
			}

			// disable the dma channel after the transfer has finished
			enabled &= ~(1 << i);
		}
	}
}

void DMA::write_dmacnt_l(u8 channel, u16 value) {
	// arm9
	if (cpu_id == 1) {
		if (value == 0) {
			// if value is 0 default to 0x200000 for arm9 dma
			dma_channel[channel].DMACNT_L = 0x200000;
		} else {
			// else have max value of 0x1FFFFF
			dma_channel[channel].DMACNT_L = value & 0x1FFFFF;
		}
		
	} else {
		// arm7
		if (value == 0) {
			// if value is 0 on arm7 then we check channel for max value
			dma_channel[channel].DMACNT_L = ((channel == 3) ? 0x10000: 0x4000);
		} else {
			// use max value depending on channel
			dma_channel[channel].DMACNT_L = ((channel == 3) ? 0xFFFF: 0x3FFF);
		}
	}
}

void DMA::write_dmacnt_h(u8 channel, u16 value) {

	// store a copy of the old dmacnt_h to check later if enable bit has been changed from 0 to 1, this will tell us if internal registers should be loaded
	u16 old_dmacnt_h = dma_channel[channel].DMACNT_H;

	// write to dmacnt_h
	dma_channel[channel].DMACNT_H = value & 0xFFE0; // bits 0..4 are unused (always 0)




	// // dont load internal registers is enable bit wasnt changed from 0 to 1
	if (get_bit(15, old_dmacnt_h) || !get_bit(15, value)) {
		return;
	}

	// otherwise load internal registers
	dma_channel[channel].internal_source_address = dma_channel[channel].DMASAD;
	dma_channel[channel].internal_destination_address = dma_channel[channel].DMADAD;
	dma_channel[channel].internal_word_count = dma_channel[channel].DMACNT_L;

	// check if bits 11..13 is 0 so that dma will occur immediately on the next cpu step
	if ((((dma_channel[channel].DMACNT_H) >> 11) & 0x7) == 0) {
		// enable the dma channel
		enabled |= (1 << channel);
	} else {
		log_fatal("other dma start timing not implemented yet: %d", ((dma_channel[channel].DMACNT_H) >> 11) & 0x7);
	}
}

bool DMA::get_dma_enabled() {
	return enabled;
}