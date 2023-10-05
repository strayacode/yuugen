#include "common/memory.h"
#include "gba/video/ppu.h"

namespace gba {

void PPU::render_mode3(int id, int line) {
    for (int x = 0; x < 240; x++) {
        int offset = ((240 * line) + x) * 2;
        bg_layers[id][x] = common::read<u16>(vram.data(), offset);
    }
}

void PPU::render_mode4(int id, int line) {
    auto bitmap_start = dispcnt.display_frame_select * 0xa000;

    for (int x = 0; x < 240; x++) {
        int offset = bitmap_start + (240 * line) + x;
        int palette_index = vram[offset];
        bg_layers[id][x] = common::read<u16>(palette_ram.data(), palette_index * 2);
    }
}

void PPU::render_mode5(int id, int line) {
    // the bitmap is only 160x128
    if (line >= 128) {
        return;
    }

    auto bitmap_start = dispcnt.display_frame_select * 0xa000;

    for (int x = 0; x < 160; x++) {
        int offset = bitmap_start + ((160 * line) + x) * 2;
        bg_layers[id][x] = common::read<u16>(vram.data(), offset);
    }
}

} // namespace gba