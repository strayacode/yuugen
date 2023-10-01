#include "nds/hardware/wifi.h"

namespace nds {

void Wifi::reset() {

}

void Wifi::write_wifiwaitcnt(u16 value, u32 mask) {
    wifiwaitcnt = (wifiwaitcnt & ~mask) | (value & mask);
}

} // namespace nds