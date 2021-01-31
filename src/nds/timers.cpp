#include <nds/nds.h>
#include <nds/timers.h>

Timers::Timers(NDS *nds, bool cpu_id) : nds(nds), cpu_id(cpu_id) {

}

void Timers::write_tmcnt_l(u8 index, u16 data) {
	// intialise the reload value ig lol
	reload_value[index] = data;
}

void Timers::write_tmcnt_h(u8 index, u16 data) {
	// if count up timing is enabled the prescalar value is ignored so instead the timer increments when the previous counter overflows? hmm
	// count up timing cant be used on timer 0 too
	
	// set bits 0..1 to the number of cycles left before tmcnt_l can increment
	switch (data & 0x3) {
	case 0:
		cycles_left[index] = 1;
	case 1:
		cycles_left[index] = 64;
	case 2:
		cycles_left[index] = 256;
	case 3:
		cycles_left[index] = 1024;
	}


	// if the timer has gone from disabled to enabled then reload tmcnt_l with the reload value
	if (!get_bit(7, tmcnt_h[index]) && get_bit(7, data)) {
		tmcnt_l[index] = reload_value[index];
	}

	// set tmcnt_h
	tmcnt_h[index] = (tmcnt_h[index] & ~0xC7) | (data & 0xC7);
	// set enable bits
	// but a counter in count up mode is disabled as the previous timer when it overflows will cause an increment in this timer
	if ((tmcnt_h[index] && (1 << 7)) && (index == 0 || !(get_bit(2, tmcnt_h[index])))) {
		enabled |= (1 << index);
	} else {
		enabled &= ~(1 << index);
	}
}

bool Timers::get_timer_enabled() {
	return enabled;
}

void Timers::tick(int cycles) {
	// if (enabled == 9) {
	// 	log_fatal("end");
	// }
	// iterate through all 4 timers for a cpu and tick only if enabled
	for (int i = 0; i < 4; i++) {
		if (!get_bit(7, tmcnt_h[i])) {
			continue;
		}

		

		// this will be decremented each time until finally when it reaches 0 the actual timer is incremented
		cycles_left[i] -= cycles;

		if (cycles_left[i] <= 0) {
			// keep a copy of the old timer value so that if new value is less then that means an overflow occured
			u16 old_timer_value = tmcnt_l[i];
			
			tmcnt_l[i]++;

			// put cycles left back to initial thing
			cycles_left[i] = tmcnt_h[i] & 0x3;

			if (tmcnt_l[i] < old_timer_value) {
				overflow(i);
			}
		}


	}
}

void Timers::overflow(u8 index) {
	// on overflow the reload value is copied into the selected tmcnt_l
	tmcnt_l[index] = reload_value[index];
	
	// check if timer overflow irqs are enabled
	if (get_bit(6, tmcnt_h[index])) {
		nds->interrupt[cpu_id].request_interrupt(3 + index);
	}

	// for the count up timing behaviour:
	// the index of this timer that overflows cant be 3 as the next index is out of range
	// also the next timer must have count up timing enabled and the timer itself must be enabled
	if (((index < 3) && (get_bit(2, tmcnt_h[index + 1])) && (get_bit(7, tmcnt_h[index + 1])))) {
		// also if the next timer is at 0xFFFF and will overflow then we also need to call overflow method on the next timer lol
		if (tmcnt_l[index + 1] == 0xFFFF) {
			overflow(index + 1);
		} else {
			// just increment next timer then
			tmcnt_l[index + 1]++;
		}
	}
}