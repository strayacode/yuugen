add_library(core
    Core.h Core.cpp
    System.h System.cpp

    ARM/MemoryBase.h ARM/MMIO.h
    ARM/ARMTypes.h ARM/ARMTypes.cpp
    ARM/cpu_core.h ARM/cpu_core.cpp
    ARM/ARM7/Memory.h ARM/ARM7/Memory.cpp
    ARM/ARM9/Memory.h ARM/ARM9/Memory.cpp
    ARM/Disassembler/Disassembler.h ARM/Disassembler/Disassembler.cpp
    ARM/Disassembler/arm.cpp ARM/Disassembler/thumb.cpp
    ARM/Decoder/Decoder.h
    
    Hardware/Cartridge/Cartridge.h Hardware/Cartridge/Cartridge.cpp
    Hardware/Cartridge/loader_base.h 
    Hardware/Cartridge/nds_loader.h Hardware/Cartridge/nds_loader.cpp
    Hardware/Cartridge/backup/flash/flash.h Hardware/Cartridge/backup/flash/flash.cpp
    Hardware/Cartridge/backup/eeprom/eeprom.h Hardware/Cartridge/backup/eeprom/eeprom.cpp
    Hardware/Cartridge/backup/no_backup/no_backup.h Hardware/Cartridge/backup/no_backup/no_backup.cpp
    Hardware/Cartridge/backup/generic_backup.h Hardware/Cartridge/save_database.h
    Hardware/SPI.h Hardware/SPI.cpp
    Scheduler.h Scheduler.cpp
    Hardware/cp15/cp15.h Hardware/cp15/cp15.cpp

    Hardware/DMA.h Hardware/DMA.cpp
    Hardware/Input.h Hardware/Input.cpp
    Hardware/IPC.h Hardware/IPC.cpp
    Hardware/Timers.h Hardware/Timers.cpp
    Hardware/SPU.h Hardware/SPU.cpp
    Hardware/RTC.h Hardware/RTC.cpp
    Hardware/MathsUnit.h Hardware/MathsUnit.cpp
    Hardware/wifi/wifi.h Hardware/wifi/wifi.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common videocommon yuugen_common audiocommon)
