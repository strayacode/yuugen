#pragma once

#include <common/types.h>

class NDS;

class RTC {
public:
	RTC(NDS *nds);

	u16 control_register;

	void reset();

private:
	NDS *nds;
};