add_library(emulator
    Emulator.cpp Emulator.h
    core/ARMInterpreter.cpp core/ARMInterpreter.h
    core/ARMInterpreter_Branch.cpp core/ARMInterpreter_Coprocessor.cpp
    core/ARMInterpreter_Exception.cpp
    core/ARMInterpreter_LoadStore.cpp core/ARMInterpreter_StatusTransfer.cpp
    core/ARMInterpreter_ALU.cpp
    core/Memory.cpp core/Memory.h
    core/CP15.cpp core/CP15.h
    core/Cartridge.cpp core/Cartridge.h
    common/types.h common/arithmetic.h
    core/GPU.cpp core/GPU.h
    core/GPU2D.cpp core/GPU2D.h
    core/DMA.cpp core/DMA.h
    common/arithmetic.cpp common/log.h
    common/mem_helpers.h
    core/Interrupt.h
    core/Keypad.h core/Keypad.cpp
)

set (DIRECT_BOOT ON)
set (FIRMWARE_BOOT OFF)
set (FILE_LOG ON)
 
if (DIRECT_BOOT)
    target_compile_definitions(emulator PRIVATE DIRECT_BOOT)
endif()

if (FIRMWARE_BOOT)
    target_compile_definitions(emulator PRIVATE FIRMWARE_BOOT)
endif()

if (FILE_LOG)
    target_compile_definitions(emulator PRIVATE FILE_LOG)
endif()



target_include_directories(emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")