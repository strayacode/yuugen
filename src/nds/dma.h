#pragma once
#include <common/types.h>
#include <common/log.h>
#include <common/arithmetic.h>

class NDS;

class DMA {
public:
	DMA(NDS *nds, bool cpu_id);

	struct dma_channel {
        u32 DMASAD; // only first 27 or 28 significant bits are used. i would this is the address that data is taken from
        
        u32 internal_source_address; // this is an internal register and DMASAD is copied into this internal register

        u32 DMADAD; // only first 27 or 28 bits are used. i would assume this specifies the address to copy data to
        
        u32 internal_destination_address; // this is an internal register and DMADAD is copied into this internal register

        u32 DMACNT_L; // specifies number of data units to be transferred with each unit being 16 bit or 32 bit depending on the transfer type
        // a value of 0 is treated as max length

        u32 internal_word_count; // this is an internal register and DMACNT_L is copied into this internal register


        // this is the control register which controls settings for each dma channel
        u16 DMACNT_H; 
    } dma_channel[4];

    void write_dmacnt_l(u8 channel, u16 value);
    void write_dmacnt_h(u8 channel, u16 value);

    bool get_dma_enabled();

    void transfer();

private:
	NDS *nds;

    bool cpu_id;

    u8 enabled = 0;
};