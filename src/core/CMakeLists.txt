add_library(core
    core.h core.cpp
    config.h config.cpp
    system.h system.cpp

    arm/memory_base.h arm/cpu_registers.h
    arm/cpu_core.h arm/cpu_core.cpp
    arm/arm7/memory.h arm/arm7/memory.cpp
    arm/arm9/memory.h arm/arm9/memory.cpp
    arm/Disassembler/Disassembler.h arm/Disassembler/Disassembler.cpp
    arm/Disassembler/arm.cpp arm/Disassembler/thumb.cpp
    arm/decoder/decoder.h
    
    hw/cartridge/cartridge.h hw/cartridge/cartridge.cpp
    hw/cartridge/loader_base.h 
    hw/cartridge/nds_loader.h hw/cartridge/nds_loader.cpp
    hw/cartridge/backup/flash/flash.h hw/cartridge/backup/flash/flash.cpp
    hw/cartridge/backup/eeprom/eeprom.h hw/cartridge/backup/eeprom/eeprom.cpp
    hw/cartridge/backup/no_backup/no_backup.h hw/cartridge/backup/no_backup/no_backup.cpp
    hw/cartridge/backup/generic_backup.h hw/cartridge/save_database.h
    hw/spi/spi.h hw/spi/spi.cpp
    scheduler/scheduler.h scheduler/scheduler.cpp
    hw/cp15/cp15.h hw/cp15/cp15.cpp
    hw/gpu/gpu.h hw/gpu/gpu.cpp
    hw/gpu/VRAMPage.h hw/gpu/VRAMRegion.h
    hw/gpu/engine_2d/gpu_2d.h hw/gpu/engine_2d/gpu_2d.cpp
    hw/gpu/engine_2d/render/text.cpp hw/gpu/engine_2d/render/extended.cpp
    hw/gpu/engine_2d/compose.cpp hw/gpu/engine_2d/render/object.cpp
    hw/gpu/engine_2d/render/affine.cpp hw/gpu/engine_2d/render/large.cpp
    
    hw/gpu/engine_3d/render_engine.h hw/gpu/engine_3d/render_engine.cpp
    hw/gpu/engine_3d/geometry_engine.h hw/gpu/engine_3d/geometry_engine.cpp
    hw/gpu/engine_3d/geometry_commands.cpp
    hw/gpu/engine_3d/Interpolator.h

    hw/dma/dma.h hw/dma/dma.cpp
    hw/input/input.h hw/input/input.cpp
    hw/ipc/ipc.h hw/ipc/ipc.cpp
    hw/timers/timers.h hw/timers/timers.cpp
    hw/spu/spu.h hw/spu/spu.cpp
    hw/rtc/rtc.h hw/rtc/rtc.cpp
    hw/maths_unit/maths_unit.h hw/maths_unit/maths_unit.cpp
    hw/wifi/wifi.h hw/wifi/wifi.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common yuugen_common audio_common)
