#pragma once

#include "common/types.h"

namespace nds {

class GPU {
public:
    void reset();
    void write_disp3dcnt(u32 value, u32 mask);

private:
    union DISP3DCNT {
        struct {
            bool texture_mapping : 1;
            bool polygon_shading : 1;
            bool alpha_test : 1;
            bool alpha_blending : 1;
            bool anti_aliasing : 1;
            bool edge_marking : 1;
            bool alpha_mode : 1;
            bool fog_enable : 1;
            u32 fog_depth_shift : 4;
            bool rdlines_underflow : 1;
            bool polygon_vertex_ram_overflow : 1;
            bool rear_plane_mode : 1;
            u32 : 17;
        };

        u32 data;
    };

    DISP3DCNT disp3dcnt;
};

} // namespace nds