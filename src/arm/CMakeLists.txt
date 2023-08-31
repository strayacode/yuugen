add_library(arm
    arch.h
    coprocessor.h
    memory.h
    cpu.h state.h

    interpreter/interpreter.h interpreter/interpreter.cpp
    interpreter/instructions/alu.cpp interpreter/instructions/arm.cpp
    interpreter/instructions/thumb.cpp

    jit/jit.h jit/jit.cpp
    jit/basic_block.h
    jit/location.h jit/code_cache.h
    jit/config.h
    jit/ir/emitter.h jit/ir/emitter.cpp
    jit/ir/translator.h jit/ir/translator.cpp
    jit/ir/opcodes.h jit/ir/types.h
    jit/ir/instructions/arm.cpp jit/ir/instructions/thumb.cpp
    jit/backend/backend.h
    jit/backend/ir_interpreter/ir_interpreter.h jit/backend/ir_interpreter/ir_interpreter.cpp

    disassembler/disassembler.h disassembler/disassembler.cpp
    disassembler/arm.cpp disassembler/thumb.cpp

    decoder.h instructions.h
)

include_directories(arm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(arm PUBLIC common)
