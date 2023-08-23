#include "core/hardware/wifi.h"

namespace core {

void Wifi::reset() {

}

void Wifi::write_wifiwaitcnt(u16 value, u32 mask) {
    wifiwaitcnt = (wifiwaitcnt & ~mask) | (value & mask);
}

} // namespace core