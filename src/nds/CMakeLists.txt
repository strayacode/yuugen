add_library(nds
	nds.h nds.cpp
	arm.h arm.cpp
	gpu.h gpu.cpp
	gpu_2d.h gpu_2d.cpp
	memory.h memory.cpp
	cartridge.h cartridge.cpp
	interrupt.h interrupt.cpp
	ipc.h ipc.cpp
	cp15.h cp15.cpp
	rtc.h rtc.cpp
	timers.h timers.cpp
	spi.h spi.cpp
	spu.h spu.cpp
	dma.h dma.cpp
	input.h input.cpp

	arm_branch.cpp
	arm_alu.cpp
	arm_transfer.cpp




)

set (IO_DEBUG OFF)

if (IO_DEBUG) 
	target_compile_definitions(nds PRIVATE IO_DEBUG)
endif()

target_include_directories(nds PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(nds PRIVATE common)