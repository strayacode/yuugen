#include <core/core.h>
#include <core/spi.h>

SPI::SPI(Core* core) : core(core) {

}

void SPI::Reset() {
    SPICNT = 0;
    SPIDATA = 0;
}

void SPI::WriteSPICNT(u16 data) {
    SPICNT = (SPICNT & ~0xCF03) | (data & 0xCF03);
}

void SPI::WriteSPIDATA(u8 data) {
    // TODO: properly handle behaviour of spi
    SPIDATA = data;
}