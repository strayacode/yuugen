add_library(arm
    arch.h
    coprocessor.h null_coprocessor.h
    memory.h
    cpu.h state.h
    arithmetic.h arithmetic.cpp

    interpreter/interpreter.h interpreter/interpreter.cpp
    interpreter/instructions/alu.cpp interpreter/instructions/arm.cpp
    interpreter/instructions/thumb.cpp

    jit/jit.h jit/jit.cpp
    jit/basic_block.h jit/config.h
    jit/location.h
    jit/ir/ir_emitter.h jit/ir/ir_emitter.cpp
    jit/ir/translator.h jit/ir/translator.cpp
    jit/ir/optimiser.h jit/ir/optimiser.cpp

    jit/ir/pass.h
    jit/ir/passes/dead_load_store_elimination_pass.h jit/ir/passes/dead_load_store_elimination_pass.cpp
    jit/ir/passes/dead_copy_elimination_pass.h jit/ir/passes/dead_copy_elimination_pass.cpp
    jit/ir/passes/const_propagation_pass.h jit/ir/passes/const_propagation_pass.cpp
    jit/ir/passes/identity_arithmetic_pass.h jit/ir/passes/identity_arithmetic_pass.cpp

    jit/ir/opcodes.h jit/ir/value.h
    jit/ir/translate/arm.cpp jit/ir/translate/thumb.cpp
    jit/backend/backend.h jit/backend/code_cache.h
    jit/backend/code.h
    jit/backend/ir_interpreter/ir_interpreter.h jit/backend/ir_interpreter/ir_interpreter.cpp

    jit/backend/a64/backend.h jit/backend/a64/backend.cpp
    jit/backend/a64/assembler.h jit/backend/a64/assembler.cpp
    jit/backend/a64/register.h jit/backend/a64/code_block.h
    jit/backend/a64/register_allocator.h jit/backend/a64/register_allocator.cpp
    jit/backend/a64/disassembler.h jit/backend/a64/disassembler.cpp

    disassembler/disassembler.h disassembler/disassembler.cpp
    disassembler/arm.cpp disassembler/thumb.cpp

    decoder.h instructions.h
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(arm PUBLIC ${LLVM_INCLUDE_DIRS})
link_directories(arm PUBLIC ${LLVM_LIBRARY_DIRS})

llvm_map_components_to_libnames(llvm_libs armdesc armdisassembler aarch64desc aarch64disassembler)

include_directories(arm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(arm PUBLIC common ${llvm_libs})