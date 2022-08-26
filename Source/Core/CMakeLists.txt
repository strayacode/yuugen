add_library(core
    Core.h Core.cpp
    System.h System.cpp
    Scheduler.h Scheduler.cpp

    ARM/MemoryBase.h ARM/MMIO.h
    ARM/ARMTypes.h ARM/ARMTypes.cpp
    ARM/ARM7/Memory.h ARM/ARM7/Memory.cpp
    ARM/ARM9/Memory.h ARM/ARM9/Memory.cpp
    ARM/ARM7/Coprocessor.h ARM/ARM7/Coprocessor.cpp
    ARM/ARM9/Coprocessor.h ARM/ARM9/Coprocessor.cpp
    ARM/ARM7/ARM7.h ARM/ARM7/ARM7.cpp
    ARM/ARM9/ARM9.h ARM/ARM9/ARM9.cpp
    ARM/Disassembler/Disassembler.h ARM/Disassembler/Disassembler.cpp
    ARM/Disassembler/arm.cpp ARM/Disassembler/thumb.cpp
    ARM/Decoder.h
    ARM/CPUBase.h ARM/CPUBase.cpp
    ARM/CoprocessorBase.h
    ARM/Interpreter/Interpreter.h ARM/Interpreter/Interpreter.cpp
    ARM/Interpreter/Instructions/ALU.cpp ARM/Interpreter/Instructions/Branch.cpp
    ARM/Interpreter/Instructions/Transfer.cpp
    
    Hardware/Cartridge/Cartridge.h Hardware/Cartridge/Cartridge.cpp
    Hardware/Cartridge/loader_base.h 
    Hardware/Cartridge/nds_loader.h Hardware/Cartridge/nds_loader.cpp
    Hardware/Cartridge/backup/flash/flash.h Hardware/Cartridge/backup/flash/flash.cpp
    Hardware/Cartridge/backup/eeprom/eeprom.h Hardware/Cartridge/backup/eeprom/eeprom.cpp
    Hardware/Cartridge/backup/no_backup/no_backup.h Hardware/Cartridge/backup/no_backup/no_backup.cpp
    Hardware/Cartridge/backup/generic_backup.h Hardware/Cartridge/save_database.h
    Hardware/SPI.h Hardware/SPI.cpp
    Hardware/DMA.h Hardware/DMA.cpp
    Hardware/Input.h Hardware/Input.cpp
    Hardware/IPC.h Hardware/IPC.cpp
    Hardware/Timers.h Hardware/Timers.cpp
    Hardware/SPU.h Hardware/SPU.cpp
    Hardware/RTC.h Hardware/RTC.cpp
    Hardware/MathsUnit.h Hardware/MathsUnit.cpp    
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common videocommon yuugen_common audiocommon)
