add_library(core
    core.h core.cpp
    scheduler.h scheduler.cpp
    system.h

    arm/arch.h
    arm/coprocessor.h
    arm/virtual_page_table.h
    arm/memory.h
    arm/cpu.h arm/state.h
    arm/interpreter/interpreter.h arm/interpreter/interpreter.cpp
    arm/interpreter/instructions/alu.cpp arm/interpreter/instructions/arm.cpp
    arm/interpreter/instructions/thumb.cpp
    arm/disassembler/disassembler.h arm/disassembler/disassembler.cpp
    arm/disassembler/arm.cpp arm/disassembler/thumb.cpp
    arm/decoder.h arm/instructions.h

    nds/system.h nds/system.cpp
    nds/config.h

    nds/arm7/arm7.h nds/arm7/arm7.cpp
    nds/arm7/memory.h nds/arm7/memory.cpp
    nds/arm7/coprocessor.h nds/arm7/coprocessor.cpp

    nds/arm9/arm9.h nds/arm9/arm9.cpp
    nds/arm9/memory.h nds/arm9/memory.cpp
    nds/arm9/coprocessor.h nds/arm9/coprocessor.cpp

    nds/video/video_unit.h nds/video/video_unit.cpp
    nds/video/vram.h nds/video/vram.cpp
    nds/video/vram_region.h nds/video/vram_page.h
    nds/video/ppu.h nds/video/ppu.cpp

    nds/hardware/cartridge/cartridge.h nds/hardware/cartridge/cartridge.cpp
    nds/hardware/input.h nds/hardware/input.cpp
    nds/hardware/spu.h nds/hardware/spu.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(core PRIVATE Threads::Threads)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common)
