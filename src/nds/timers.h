#pragma once

#include <common/types.h>
#include <common/arithmetic.h>
#include <common/log.h>

class NDS;

// the NDS has 4 timers for each cpu, so 8 timers in total


class Timers {
public:
	Timers(NDS *nds);

	void write_tmcnt_l(u8 index, u16 data);
	void write_tmcnt_h(u8 index, u16 data);
	bool get_timer_enabled();
	void tick(int cycles);
	void overflow(u8 index);

private:
	NDS *nds;

	// the nds for each cpu has 4 incrementing 16 bit timers

	// writing to these registers initialises the reload value but doesnt directly affect the timer value
	// reading returns the current timer value
	// the reload value is copied into the timer value when: timer overflows or when the timer start bit changes from 0 to 1
	u16 tmcnt_l[4] = {};

	// have 4 reload values that correspond to each tmcnt_l
	u16 reload_value[4] = {};

	// this is changed in tmcnt_h writes to the number of cycles needed before a timer can increment
	u16 cycles_left[4] = {};

	u8 enabled = 0;

	// each register corresponds to some settings for each tmcnt_l
	u16 tmcnt_h[4] = {};
};