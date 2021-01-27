#include <nds/nds.h>

// TODO: add cpu id to dma later
NDS::NDS() : spi(this), cp15(this), maths_unit(this), spu(this), dma {DMA(this, 0), DMA(this, 1)}, arm9(this, 1), arm7(this, 0), timers {Timers(this, 0), Timers(this, 1)}, gpu(this), memory(this), cartridge(this), interrupt {Interrupt(this, 0), Interrupt(this, 0)}, ipc(this), rtc(this) {
	
}

// TODO: add rom path later but we will try to boot the firmware without a cartridge inserted for now
void NDS::firmware_boot() {
    reset();

	// load the 2 bioses and the firmware
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    spi.load_firmware();
    
    // firmware boot the arm9 and arm7
    arm9.firmware_boot();
    arm7.firmware_boot();
}

void NDS::run_nds_frame() {
	// quick sidenote
    // in 1 frame of the nds executing
    // there are 263 scanlines with 192 visible and 71 for vblank
    // in each scanline there are 355 dots in total with 256 visible and 99 for hblank
    // 3 cycles of the arm7 occurs per dot and 6 cycles of the arm9 occurs per dot
    for (int i = 0; i < 263; i++) {
        for (int j = 0; j < 355 * 3; j++) {
            // run arm9 and arm7 stuff
            // since arm9 runs at the twice the clock speed of arm7 we run it 2 times instead of 1
            for (int i = 0; i < 2; i++) {
                arm9.step();
                if (timers[1].get_timer_enabled()) {
                    // tick arm9 timers by 2 cycles
                    timers[1].tick(1);
                }
                if (dma[1].get_dma_enabled()) {
                    dma[1].transfer();
                }
            }
            
        
            arm7.step();
            if (timers[0].get_timer_enabled()) {
                // tick arm7 timers by 2 cycles
                timers[0].tick(2);
            }
            if (dma[0].get_dma_enabled()) {
                dma[0].transfer();
            }

            // this is called for hblank to begin when 256 dots have passed in a scanline (each dot is 3 cycle)
            if (j == 768) {
                gpu.render_scanline_begin(i);
            }      
        }
        gpu.render_scanline_end(i);
    }
}

void NDS::reset() {
    // TODO: add more reset state stuff from other components
    memory.reset();
    ipc.reset();
    rtc.reset();
}

void NDS::direct_boot(std::string rom_path) {
    cartridge.load_cartridge(rom_path);

    // reflect specific memory that is changed by the firmware boot
    // set io registers
    memory.WRAMCNT = 0x03;

    // set POSTFLG9, POSTFLG7 to 1 to show that the firmware boot has finished
    memory.POSTFLG9 = memory.POSTFLG7 = 0x1;

    gpu.POWCNT1 = 0x1;

    // set values some values in main memory
    memory.arm7_write_word(0x027FF800, 0x00003FC2); // chip ID 1
    memory.arm7_write_word(0x027FF804, 0x00003FC2); // chip ID 2


    // i dont even know if this stuff is needed lmao
    memory.arm7_write_halfword(0x027FF808, cartridge.rom[0x15E]); // cartridge header crc
    memory.arm7_write_halfword(0x027FF80A, cartridge.rom[0x6C]); // cartridge secure area crc 

    memory.arm7_write_halfword(0x027FF850, 0x5835); // arm7 bios crc
    memory.arm7_write_halfword(0x027FF880, 0x0007); // message from arm9 to arm7
    memory.arm7_write_halfword(0x027FF884, 0x0006); // ARM7 boot task
    memory.arm7_write_word(0x027FFC00, 0x3FC2); // copy of chip ID 1
    memory.arm7_write_word(0x027FFC04, 0x3FC2); // copy of chip ID 2
    memory.arm7_write_halfword(0x027FFC10, 0x5835); // copy of arm7 bios crc
    memory.arm7_write_halfword(0x027FFC40, 0x0001); // boot indicator


    cartridge.transfer_rom();

    cp15.direct_boot();

    // the bioses and firmware
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    spi.load_firmware();

    // direct boot the arm9 and arm7
    arm9.direct_boot();
    arm7.direct_boot();
    spi.direct_boot();
}