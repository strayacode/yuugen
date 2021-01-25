#include <nds/dma.h>
#include <nds/nds.h>

DMA::DMA(NDS *nds, int cpu_id) : nds(nds), cpu_id(cpu_id) {

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


	printf("new value for dmacnt_h: 0x%04x\n", value);
	printf("old dmacnt_h: 0x%04x\n", old_dmacnt_h);
	// write to dmacnt_h
	dma_channel[channel].DMACNT_H = value & 0xFFE0; // bits 0..4 are unused (always 0)




	// // dont load internal registers is enable bit wasnt changed from 0 to 1
	if (get_bit(15, old_dmacnt_h) || !get_bit(15, value)) {
		return;
	}


	// just exit because we havent implemented dma transfers lol
	log_fatal("dma transfers not implemented yet!");

	// otherwise load internal registers
	dma_channel[channel].internal_source_address = dma_channel[channel].DMASAD;
	dma_channel[channel].internal_destination_address = dma_channel[channel].DMADAD;
	dma_channel[channel].internal_word_count = dma_channel[channel].DMACNT_L;
}