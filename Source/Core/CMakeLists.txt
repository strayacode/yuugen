add_library(core
    core.h core.cpp
    system.h system.cpp

    arm/memory_base.h arm/cpu_registers.h
    arm/cpu_core.h arm/cpu_core.cpp
    arm/arm7/memory.h arm/arm7/memory.cpp
    arm/arm9/memory.h arm/arm9/memory.cpp
    arm/Disassembler/Disassembler.h arm/Disassembler/Disassembler.cpp
    arm/Disassembler/arm.cpp arm/Disassembler/thumb.cpp
    arm/Decoder/Decoder.h
    
    HW/cartridge/cartridge.h HW/cartridge/cartridge.cpp
    HW/cartridge/loader_base.h 
    HW/cartridge/nds_loader.h HW/cartridge/nds_loader.cpp
    HW/cartridge/backup/flash/flash.h HW/cartridge/backup/flash/flash.cpp
    HW/cartridge/backup/eeprom/eeprom.h HW/cartridge/backup/eeprom/eeprom.cpp
    HW/cartridge/backup/no_backup/no_backup.h HW/cartridge/backup/no_backup/no_backup.cpp
    HW/cartridge/backup/generic_backup.h HW/cartridge/save_database.h
    HW/spi/spi.h HW/spi/spi.cpp
    scheduler/scheduler.h scheduler/scheduler.cpp
    HW/cp15/cp15.h HW/cp15/cp15.cpp

    HW/dma/dma.h HW/dma/dma.cpp
    HW/input/input.h HW/input/input.cpp
    HW/ipc/ipc.h HW/ipc/ipc.cpp
    HW/timers/timers.h HW/timers/timers.cpp
    HW/spu/spu.h HW/spu/spu.cpp
    HW/rtc/rtc.h HW/rtc/rtc.cpp
    HW/maths_unit/maths_unit.h HW/maths_unit/maths_unit.cpp
    HW/wifi/wifi.h HW/wifi/wifi.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common videocommon yuugen_common audiocommon)
