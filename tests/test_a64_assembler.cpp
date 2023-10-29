#include <cassert>
#include "common/logger.h"
#include "arm/jit/backend/a64/register.h"
#include "arm/jit/backend/a64/assembler.h"

void check_values(const char *testcase, u32 expected, u32 actual) {
    if (expected != actual) {
        logger.error("%s expected: %08x got: %08x", testcase, expected, actual);
    } else {
        logger.debug("%s passed", testcase);
    }
}

int main() {
    arm::A64Assembler assembler;

    #define STRINGIZE(x) #x

    #define TEST(command, expected)                                    \
        assembler.command;                                          \
        check_values(STRINGIZE(command), expected, assembler.get_code().back());    \

    TEST(ret(), 0xd65f03c0);
    TEST(ret(arm::XReg{3}), 0xd65f0060);
    TEST(ret(arm::XReg{20}), 0xd65f0280);

    return 0;
}