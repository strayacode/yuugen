#pragma once
#include <common/types.h>


class NDS;

class DMA {
public:
	DMA(NDS *nds);

	struct dma_channel {
        u32 DMASAD; // only first 27 or 28 significant bits are used. i would this is the address that data is taken from
        
        u32 DMADAD; // only first 27 or 28 bits are used. i would assume this specifies the address to copy data to
        u16 DMACNT_L; // specifies number of data units to be transferred with each unit being 16 bit or 32 bit depending on the transfer type
        // a value of 0 is treated as max length


        // this is the control register which controls settings for each dma channel
        u16 DMACNT_H; 
    } channel[4];

private:
	NDS *nds;
};