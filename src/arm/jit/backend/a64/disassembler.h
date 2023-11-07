#pragma once

#include <string>
#include "common/types.h"

namespace arm {

std::string disassemble_a64_instruction(u64 pc, u32 instruction);

} // namespace