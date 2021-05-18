#include <core/hw/rtc/rtc.h>
#include <core/core.h>


RTC::RTC(Core* core) : core(core) {

}

void RTC::Reset() {
    RTC_REG = 0;
}

auto RTC::ReadRTC() -> u8 {
    return RTC_REG;
}

void RTC::WriteRTC(u8 data) {
    RTC_REG = data;
}