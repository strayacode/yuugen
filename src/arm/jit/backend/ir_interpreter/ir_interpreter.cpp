#include "common/logger.h"
#include "arm/jit/backend/ir_interpreter/ir_interpreter.h"

namespace arm {

void IRInterpreter::reset() {
    code_cache.reset();
}

bool IRInterpreter::has_code_at(Location location) {
    return code_cache.has_code_at(location);
}

void IRInterpreter::compile(BasicBlock& basic_block)  {
    logger.todo("IRInterpreter: handle compile");
}

void IRInterpreter::run(Location location)  {
    logger.todo("IRInterpreter: handle run");
}

} // namespace arm