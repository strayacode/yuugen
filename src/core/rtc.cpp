#include <core/core.h>
#include <core/rtc.h>

RTC::RTC(Core* core) : core(core) {

}

void RTC::Reset() {
    RTC_REG = 0;
}

u8 RTC::ReadRTC() {
    return RTC_REG;
}

void RTC::WriteRTC(u8 data) {
    RTC_REG = data;
}