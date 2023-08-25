add_library(arm
    arch.h
    coprocessor.h
    memory.h
    cpu.h state.h

    interpreter/interpreter.h interpreter/interpreter.cpp
    interpreter/instructions/alu.cpp interpreter/instructions/arm.cpp
    interpreter/instructions/thumb.cpp

    jit/jit.h jit/jit.cpp
    jit/basic_block.h jit/block_cache.h
    jit/ir/opcodes.h

    disassembler/disassembler.h disassembler/disassembler.cpp
    disassembler/arm.cpp disassembler/thumb.cpp

    decoder.h instructions.h
)

include_directories(arm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(arm PUBLIC common)
