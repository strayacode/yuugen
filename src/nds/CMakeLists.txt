add_library(nds
    system.h system.cpp
    
    arm7/arm7.h arm7/arm7.cpp
    arm7/memory.h arm7/memory.cpp
    arm7/mmio.cpp
    
    arm9/arm9.h arm9/arm9.cpp
    arm9/memory.h arm9/memory.cpp
    arm9/mmio.cpp
    arm9/coprocessor.h arm9/coprocessor.cpp

    video/video_unit.h video/video_unit.cpp
    video/vram.h video/vram.cpp
    video/vram_region.h video/vram_page.h
    video/ppu/ppu.h video/ppu/ppu.cpp
    video/ppu/affine.cpp video/ppu/composer.cpp
    video/ppu/object.cpp video/ppu/text.cpp 
    video/ppu/tile_decoder.cpp
    video/gpu/gpu.h video/gpu/gpu.cpp
    video/gpu/commands.cpp
    video/gpu/matrix.h video/gpu/matrix_stack.h
    video/gpu/vertex.h video/gpu/polygon.h
    video/gpu/backend/renderer.h
    video/gpu/backend/software/interpolator.h
    video/gpu/backend/software/texture_decoder.cpp
    video/gpu/backend/software/software_renderer.h video/gpu/backend/software/software_renderer.cpp

    hardware/cartridge/cartridge.h hardware/cartridge/cartridge.cpp
    hardware/cartridge/save_database.h
    hardware/cartridge/backup/backup.h
    hardware/cartridge/backup/no_backup.h hardware/cartridge/backup/no_backup.cpp
    hardware/cartridge/backup/flash_backup.h hardware/cartridge/backup/flash_backup.cpp
    hardware/cartridge/backup/eeprom_backup.h hardware/cartridge/backup/eeprom_backup.cpp
    hardware/input.h hardware/input.cpp
    hardware/spu.h hardware/spu.cpp
    hardware/dma.h hardware/dma.cpp
    hardware/ipc.h hardware/ipc.cpp
    hardware/irq.h hardware/irq.cpp
    hardware/maths_unit.h hardware/maths_unit.cpp
    hardware/rtc.h hardware/rtc.cpp
    hardware/spi.h hardware/spi.cpp
    hardware/timers.h hardware/timers.cpp
    hardware/wifi.h hardware/wifi.cpp
)

include_directories(nds PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(nds PUBLIC arm common)
