#include <nds/nds.h>
#include <nds/rtc.h>

RTC::RTC(NDS *nds) : nds(nds) {
	
}

void RTC::reset() {
	control_register = 0;
}