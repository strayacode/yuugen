#include "gba/video/ppu.h"

namespace gba {

void PPU::render_mode0(int id, int line) {
    logger.todo("handle bg rendering");

    if (bgcnt[id].mosaic) {
        logger.todo("handle mosaic");
    }

    int y = (line + bgvofs[id]) % 512;
}

} // namespace gba