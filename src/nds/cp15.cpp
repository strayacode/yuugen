#include <nds/nds.h>
#include <nds/cp15.h>

CP15::CP15(NDS *nds) : nds(nds) {

}

void CP15::direct_boot() {
    // configure control register (enable some stuff including dtcm and itcm and instruction cache) (bits 3..6 are always 1 and bits 0, 2, 7, 12..19 are r/w) the rest are 0
    write_reg(1, 0, 0, 0x0005707D);

    // configure data tcm base
    // details:
    // bits 1..5 specify virtual size as 512 << (N) or 512 SHL N. minimum is 3 (4kb) and maximum is 23 (4gb)
    // bits 12..31 
    write_reg(9, 1, 0, 0x0300000A);

    // configure instruction tcm size
    write_reg(9, 1, 1, 0x00000020);

    log_debug("sucessfully initialised cp15 direct boot state");
}


u32 CP15::read_reg(u32 cn, u32 cm, u32 cp) {
	// log_warn("reading from register C%d, C%d, C%d", cn, cm, cp);
	switch (cn << 16 | cm << 8 | cp) {
	case 0x010000:
		// read control register
		return control_register;
	case 0x090100:
		// read dtcm base and virtual size
		return dtcm_reg;
	default:
        log_fatal("[CP15] undefined register read C%d, C%d, C%d", cn, cm, cp);
	}
}

void CP15::write_reg(u32 cn, u32 cm, u32 cp, u32 data) {
	// log_warn("writing data 0x%08x to register C%d, C%d, C%d", data, cn, cm, cp);
	switch (cn << 16 | cm << 8 | cp) {
	case 0x010000:
		control_register = data;
		break;
    case 0x020000:
        // PU Cachability Bits for Data/Unified Protection Region
    case 0x020001:
        // PU Cachability Bits for Instruction Protection Region
    case 0x030000:
        // PU Cache Write-Bufferability Bits for Data Protection Regions
        break;
    case 0x050002:
        // PU Extended Access Permission Data/Unified Protection Region
        break;
    case 0x050003:
        // PU Extended Access Permission Instruction Protection Region
        break;
    case 0x060000: case 0x060100: case 0x060200: case 0x060300: case 0x060400: case 0x060500: case 0x060600: case 0x060700:
        // dont do anything lol this is just protection unit region stuff which we dont need to emulate
        break;
	case 0x070500:
		// invalidate entire instruction cache
		log_warn("[CP15] invalidating the entire instruction cache");
        memset(instruction_cache, 0, 0x2000);
        break;
    case 0x070600:
        // invalidate the data cache (clear of forget the data i think)
        log_warn("[CP15] invalidating the entire data cache");
        memset(data_cache, 0, 0x1000);
        break;
    case 0x070A04:
        // drain the write buffer
        // this doesnt matter much since it seems to be pu related
        log_warn("[CP15] draining the writebuffer");
        break;
    case 0x090100:
    	// set dtcm size and base
    	dtcm_reg = data;

    	// set the base address
    	dtcm_base_addr = dtcm_reg >> 12;
    	// dtcm base shl 12
    	dtcm_base_addr <<= 12;

    	// set the size
    	dtcm_size = (dtcm_reg >> 1) & 0x1F;

    	// dtcm size 512 shl n
    	dtcm_size = 512 << dtcm_size;

    	log_warn("dtcm size: 0x%04x dtcm_base_addr: 0x%04x", dtcm_size, dtcm_base_addr);
    	break;
    case 0x090101:
        // write to raw register
        itcm_reg = data;

        // change itcm_size accordingly
        itcm_size = (itcm_reg >> 1) & 0x1F;

        // itcm size 512 shl n
        itcm_size = 512 << itcm_size;
        
        log_warn("itcm size: 0x%04x itcm_base_addr: 0x%04x", itcm_size, itcm_base_addr);
        // but itcm_base_addr always remains 0x00000000 so we will not change that
        break;
	default:
        log_fatal("[CP15] undefined register write C%d, C%d, C%d", cn, cm, cp);
	}
}

bool CP15::get_dtcm_enabled() {
	return (get_bit(16, control_register));
}

bool CP15::get_itcm_enabled() {
	return (get_bit(18, control_register));
}

u32 CP15::get_dtcm_base_addr() {
	return dtcm_base_addr;
}

u32 CP15::get_dtcm_size() {
	return dtcm_size;
}

u32 CP15::get_itcm_size() {
	return itcm_size;
}

u32 CP15::get_exception_base() {
    return (get_bit(13, control_register) ? 0xFFFF0000 : 0x00000000);
}