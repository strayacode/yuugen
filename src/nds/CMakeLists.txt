add_library(nds
    nds.cpp nds.h
    core/arm7.cpp core/arm7.h
    core/arm9.cpp core/arm9.h
    core/arm7_arm_instrs.cpp core/arm7_thumb_instrs.cpp
    core/arm9_arm_instrs.cpp core/arm9_thumb_instrs.cpp
    core/memory.cpp core/memory.h
    core/cp15.cpp core/cp15.h
    core/cartridge.cpp core/cartridge.h
    common/types.h common/arithmetic.h
    common/arithmetic.cpp
)

set (DIRECT_BOOT ON)
set (FIRMWARE_BOOT OFF)
 
if (DIRECT_BOOT)
    target_compile_definitions(nds PRIVATE DIRECT_BOOT)
endif()

if (FIRMWARE_BOOT)
    target_compile_definitions(nds PRIVATE FIRMWARE_BOOT)
endif()



target_include_directories(nds PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")