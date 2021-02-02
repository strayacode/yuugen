add_library(common
	types.h log.h
	arithmetic.h
	mem_helpers.h
	emu_core.h
)
set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")