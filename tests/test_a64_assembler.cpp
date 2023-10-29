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

    #define TEST(expected, command)                                                 \
        assembler.command;                                                          \
        check_values(STRINGIZE(command), expected, assembler.get_buffer().back());  \

    // credit to oaknut for the tests
    TEST(0xd65f03c0, ret())
    TEST(0xd65f0220, ret(arm::x17))
    TEST(0xd65f0080, ret(arm::x4))
    TEST(0xd65f0300, ret(arm::x24))

    TEST(0x28977de9, stp(arm::w9, arm::wzr, arm::x15, arm::IndexMode::Post, 184))
    TEST(0x28ab5ae0, stp(arm::w0, arm::w22, arm::x23, arm::IndexMode::Post, -168))
    TEST(0xa8803598, stp(arm::x24, arm::x13, arm::x12, arm::IndexMode::Post, 0))
    TEST(0xa8a15609, stp(arm::x9, arm::x21, arm::x16, arm::IndexMode::Post, -496))
    TEST(0x29a80797, stp(arm::w23, arm::w1, arm::x28, arm::IndexMode::Pre, -192))
    TEST(0x29b84e42, stp(arm::w2, arm::w19, arm::x18, arm::IndexMode::Pre, -64))
    TEST(0x2987acc3, stp(arm::w3, arm::w11, arm::x6, arm::IndexMode::Pre, 60))
    TEST(0xa99a78fa, stp(arm::x26, arm::x30, arm::x7, arm::IndexMode::Pre, 416))
    TEST(0xa9abf070, stp(arm::x16, arm::x28, arm::x3, arm::IndexMode::Pre, -328))
    TEST(0xa9bb7b8d, stp(arm::x13, arm::x30, arm::x28, arm::IndexMode::Pre, -80))
    TEST(0x293a6611, stp(arm::w17, arm::w25, arm::x16, -48))
    TEST(0x293bdc3e, stp(arm::w30, arm::w23, arm::x1, -36))
    TEST(0x292791a2, stp(arm::w2, arm::w4, arm::x13, -196))
    TEST(0xa922272d, stp(arm::x13, arm::x9, arm::x25, -480))
    TEST(0xa9222d40, stp(arm::x0, arm::x11, arm::x10, -480))
    TEST(0xa90017ca, stp(arm::x10, arm::x5, arm::x30))

    return 0;
}