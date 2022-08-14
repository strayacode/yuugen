add_library(core
    Core.h Core.cpp
    System.h System.cpp

    ARM/memory_base.h ARM/cpu_registers.h
    ARM/MMIO.h
    ARM/ARMTypes.h ARM/ARMTypes.cpp
    ARM/cpu_core.h ARM/cpu_core.cpp
    ARM/arm7/memory.h ARM/arm7/memory.cpp
    ARM/arm9/memory.h ARM/arm9/memory.cpp
    ARM/Disassembler/Disassembler.h ARM/Disassembler/Disassembler.cpp
    ARM/Disassembler/arm.cpp ARM/Disassembler/thumb.cpp
    ARM/Decoder/Decoder.h
    
    Hardware/Cartridge/cartridge.h Hardware/Cartridge/cartridge.cpp
    Hardware/Cartridge/loader_base.h 
    Hardware/Cartridge/nds_loader.h Hardware/Cartridge/nds_loader.cpp
    Hardware/Cartridge/backup/flash/flash.h Hardware/Cartridge/backup/flash/flash.cpp
    Hardware/Cartridge/backup/eeprom/eeprom.h Hardware/Cartridge/backup/eeprom/eeprom.cpp
    Hardware/Cartridge/backup/no_backup/no_backup.h Hardware/Cartridge/backup/no_backup/no_backup.cpp
    Hardware/Cartridge/backup/generic_backup.h Hardware/Cartridge/save_database.h
    Hardware/spi/spi.h Hardware/spi/spi.cpp
    Scheduler.h Scheduler.cpp
    Hardware/cp15/cp15.h Hardware/cp15/cp15.cpp

    Hardware/dma/dma.h Hardware/dma/dma.cpp
    Hardware/Input.h Hardware/Input.cpp
    Hardware/IPC.h Hardware/IPC.cpp
    Hardware/timers/timers.h Hardware/timers/timers.cpp
    Hardware/spu/spu.h Hardware/spu/spu.cpp
    Hardware/rtc/rtc.h Hardware/rtc/rtc.cpp
    Hardware/maths_unit/maths_unit.h Hardware/maths_unit/maths_unit.cpp
    Hardware/wifi/wifi.h Hardware/wifi/wifi.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common videocommon yuugen_common audiocommon)
