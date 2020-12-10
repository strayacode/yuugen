add_library(dmg
    dmg.cpp dmg.h
    core/sm83.cpp core/sm83.h
    core/memory.cpp core/memory.h
    core/video.cpp core/video.h
    core/cartridge.cpp core/cartridge.h
    core/sm83_instrs.cpp
    common/types.h
)

target_include_directories(dmg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")