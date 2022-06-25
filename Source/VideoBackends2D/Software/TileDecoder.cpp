#include "Common/Memory.h"
#include "VideoCommon/VideoUnit.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

u16 SoftwareRenderer2D::decode_obj_pixel_4bpp(u32 base, int number, int x, int y) {
    u8 indices = video_unit.vram.read_vram<u8>(base + (y * 32) + (x / 2));
    u8 index = (indices >> (4 * (x & 0x1))) & 0xF;
    
    if (index == 0) {
        return 0x8000;
    } else {
        return Common::read<u16>(palette_ram, (0x200 + (number * 32) + (index * 2)) & 0x3FF);
    }
}

u16 SoftwareRenderer2D::decode_obj_pixel_8bpp(u32 base, int number, int x, int y) {
    u8 index = video_unit.vram.read_vram<u8>(base + (y * 64) + x);
    
    if (index == 0) {
        return 0x8000;
    } else if (dispcnt & (1 << 31)) {
        if (engine == Engine::A) {
            return video_unit.vram.read_ext_palette_obja<u16>((number * 0xFF + index) * 2);
        } else {
            return video_unit.vram.read_ext_palette_objb<u16>((number * 0xFF + index) * 2);
        }
    } else {
        return Common::read<u16>(palette_ram, (0x200 + (index * 2)) & 0x3FF);
    }
}