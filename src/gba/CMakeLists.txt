add_library(gba
	gba.h gba.cpp
	arm.h arm.cpp
	gpu.h gpu.cpp
	input.h input.cpp
	cartridge.h cartridge.cpp
)

target_include_directories(gba PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(gba PRIVATE common)