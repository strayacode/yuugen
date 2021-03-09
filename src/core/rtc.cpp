#include <core/core.h>
#include <core/rtc.h>

RTC::RTC(Core* core) : core(core) {

}

void RTC::Reset() {
    RTC_REG = 0;
}