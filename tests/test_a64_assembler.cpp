#include <cassert>
#include "common/logger.h"
#include "arm/jit/backend/a64/register.h"
#include "arm/jit/backend/a64/code_block.h"
#include "arm/jit/backend/a64/assembler.h"

void check_values(const char *testcase, u32 expected, u32 actual) {
    if (expected != actual) {
        logger.error("%s expected: %08x got: %08x", testcase, expected, actual);
    } else {
        logger.debug("%s passed", testcase);
    }
}

int main() {
    arm::CodeBlock code_block{1000};
    arm::A64Assembler assembler{code_block.get_code()};

    code_block.unprotect();

    #define STRINGIZE(x) #x

    #define TEST(expected, command)                                                                 \
        assembler.command;                                                                          \
        check_values(STRINGIZE(command), expected, assembler.get_code()[assembler.get_num_instructions() - 1]); \

    // credit to oaknut for the tests
    TEST(0x0b874fb9, add(arm::w25, arm::w29, arm::w7, arm::Shift::ASR, 19))
    TEST(0x0b446a2c, add(arm::w12, arm::w17, arm::w4, arm::Shift::LSR, 26))
    TEST(0x8b494f4d, add(arm::x13, arm::x26, arm::x9, arm::Shift::LSR, 19))
    TEST(0x8b5ff514, add(arm::x20, arm::x8, arm::xzr, arm::Shift::LSR, 61))

    TEST(0x0a071d7d, _and(arm::w29, arm::w11, arm::w7, arm::Shift::LSL, 7))
    TEST(0x0a5376ff, _and(arm::wzr, arm::w23, arm::w19, arm::Shift::LSR, 29))
    TEST(0x0a5900b8, _and(arm::w24, arm::w5, arm::w25, arm::Shift::LSR, 0))
    TEST(0x8ad9dc44, _and(arm::x4, arm::x2, arm::x25, arm::Shift::ROR, 55))
    TEST(0x8a872dcc, _and(arm::x12, arm::x14, arm::x7, arm::Shift::ASR, 11))
    TEST(0x8a10d18f, _and(arm::x15, arm::x12, arm::x16, arm::Shift::LSL, 52))

    TEST(0x9708781e, bl(-64888712))
    TEST(0x95b27a60, bl(113895808))
    TEST(0x956ab105, bl(95077396))

    TEST(0x28de404b, ldp(arm::w11, arm::w16, arm::x2, arm::IndexMode::Post, 240))
    TEST(0x28f72aed, ldp(arm::w13, arm::w10, arm::x23, arm::IndexMode::Post, -72))
    TEST(0x28d4aff5, ldp(arm::w21, arm::w11, arm::sp, arm::IndexMode::Post, 164))
    TEST(0xa8e14240, ldp(arm::x0, arm::x16, arm::x18, arm::IndexMode::Post, -496))
    TEST(0xa8ee6fd6, ldp(arm::x22, arm::x27, arm::x30, arm::IndexMode::Post, -288))
    TEST(0xa8c07927, ldp(arm::x7, arm::x30, arm::x9, arm::IndexMode::Post, 0))
    TEST(0x29de4596, ldp(arm::w22, arm::w17, arm::x12, arm::IndexMode::Pre, 240))
    TEST(0x29f8cee7, ldp(arm::w7, arm::w19, arm::x23, arm::IndexMode::Pre, -60))
    TEST(0xa9cd981b, ldp(arm::x27, arm::x6, arm::x0, arm::IndexMode::Pre, 216))
    TEST(0xa9f73e53, ldp(arm::x19, arm::x15, arm::x18, arm::IndexMode::Pre, -144))
    TEST(0xa9d669ae, ldp(arm::x14, arm::x26, arm::x13, arm::IndexMode::Pre, 352))
    TEST(0x297c5bc9, ldp(arm::w9, arm::w22, arm::x30, -32))
    TEST(0x29564676, ldp(arm::w22, arm::w17, arm::x19, 176))
    TEST(0x29723d75, ldp(arm::w21, arm::w15, arm::x11, -112))
    TEST(0xa946c500, ldp(arm::x0, arm::x17, arm::x8, 104))
    TEST(0xa97cbf1f, ldp(arm::xzr, arm::x15, arm::x24, -56))
    TEST(0xa952210c, ldp(arm::x12, arm::x8, arm::x8, 288))

    TEST(0xb940fb87, ldr(arm::w7, arm::x28, 248))
    TEST(0xb96d8f49, ldr(arm::w9, arm::x26, 11660))
    TEST(0xb95633e8, ldr(arm::w8, arm::sp, 5680))
    TEST(0xf95f6c29, ldr(arm::x9, arm::x1, 16088))
    TEST(0xf94a36b7, ldr(arm::x23, arm::x21, 5224))
    TEST(0xf96d8b38, ldr(arm::x24, arm::x25, 23312))

    TEST(0x53157dee, lsr(arm::w14, arm::w15, 21))
    TEST(0x53117d59, lsr(arm::w25, arm::w10, 17))
    TEST(0xd37cff9a, lsr(arm::x26, arm::x28, 60))
    TEST(0xd36afc35, lsr(arm::x21, arm::x1, 42))
    TEST(0xd37eff6d, lsr(arm::x13, arm::x27, 62))
    TEST(0x1ac32537, lsr(arm::w23, arm::w9, arm::w3))
    TEST(0x1adc268a, lsr(arm::w10, arm::w20, arm::w28))
    TEST(0x1ad027df, lsr(arm::wzr, arm::w30, arm::w16))
    TEST(0x9ac626b5, lsr(arm::x21, arm::x21, arm::x6))
    TEST(0x9acb251d, lsr(arm::x29, arm::x8, arm::x11))
    TEST(0x9ad42493, lsr(arm::x19, arm::x4, arm::x20))

    TEST(0x52a08000, mov(arm::w0, 0x04000000))
    TEST(0x2a1b03f2, mov(arm::w18, arm::w27))
    TEST(0x2a1603e5, mov(arm::w5, arm::w22))
    TEST(0x2a0c03ea, mov(arm::w10, arm::w12))
    TEST(0xaa1a03ec, mov(arm::x12, arm::x26))
    TEST(0xaa1e03e4, mov(arm::x4, arm::x30))
    TEST(0xaa1303f6, mov(arm::x22, arm::x19))

    TEST(0x72883d49, movk(arm::w9, 16874))
    TEST(0x72811bd7, movk(arm::w23, 2270))
    TEST(0x72a0d0e1, movk(arm::w1, {1671, 16}))
    TEST(0xf2b37940, movk(arm::x0, {39882, 16}))
    TEST(0xf2bfb1bf, movk(arm::xzr, {64909, 16}))
    TEST(0xf2e780fd, movk(arm::x29, {15367, 48}))

    TEST(0x52b62758, movz(arm::w24, 0xb13a0000))
    TEST(0x52b14a9e, movz(arm::w30, 0x8a540000))
    TEST(0x52837eec, movz(arm::w12, 7159))
    TEST(0xd2b5562c, movz(arm::x12, 2863726592))
    TEST(0xd2bbe6f6, movz(arm::x22, 3744923648))
    TEST(0xd2c2e35b, movz(arm::x27, 25400436588544))

    TEST(0xd51b4214, msr(arm::SystemReg::NZCV, arm::x20))
    
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

    TEST(0xb8168414, str(arm::w20, arm::x0, arm::IndexMode::Post, -152))
    TEST(0xb8090760, str(arm::w0, arm::x27, arm::IndexMode::Post, 144))
    TEST(0xb81457d6, str(arm::w22, arm::x30, arm::IndexMode::Post, -187))
    TEST(0xf80e8585, str(arm::x5, arm::x12, arm::IndexMode::Post, 232))
    TEST(0xf81907f9, str(arm::x25, arm::sp, arm::IndexMode::Post, -112))
    TEST(0xf806956a, str(arm::x10, arm::x11, arm::IndexMode::Post, 105))
    TEST(0xb809ceba, str(arm::w26, arm::x21, arm::IndexMode::Pre, 156))
    TEST(0xb8073f34, str(arm::w20, arm::x25, arm::IndexMode::Pre, 115))
    TEST(0xb8087f73, str(arm::w19, arm::x27, arm::IndexMode::Pre, 135))
    TEST(0xf81c4c08, str(arm::x8, arm::x0, arm::IndexMode::Pre, -60))
    TEST(0xf80fde06, str(arm::x6, arm::x16, arm::IndexMode::Pre, 253))
    TEST(0xf80a8c2d, str(arm::x13, arm::x1, arm::IndexMode::Pre, 168))
    TEST(0xb91ab6cb, str(arm::w11, arm::x22, 6836))
    TEST(0xb9223f0c, str(arm::w12, arm::x24, 8764))
    TEST(0xb902a76c, str(arm::w12, arm::x27, 676))
    TEST(0xf9025f66, str(arm::x6, arm::x27, 1208))
    TEST(0xf90ada92, str(arm::x18, arm::x20, 5552))
    TEST(0xf92357b6, str(arm::x22, arm::x29, 18088))

    TEST(0x5133a211, sub(arm::w17, arm::w16, arm::SubImmediate{3304}))
    TEST(0x510948af, sub(arm::w15, arm::w5, arm::SubImmediate{594}))
    TEST(0xd1088b3e, sub(arm::x30, arm::x25, arm::SubImmediate{546}))
    TEST(0xd10478ac, sub(arm::x12, arm::x5, arm::SubImmediate{286}))
    TEST(0xd12e98b0, sub(arm::x16, arm::x5, arm::SubImmediate{2982}))
    TEST(0x4b8b06d7, sub(arm::w23, arm::w22, arm::w11, arm::Shift::ASR, 1))
    TEST(0xcb4adab1, sub(arm::x17, arm::x21, arm::x10, arm::Shift::LSR, 54))
    TEST(0xcb9656de, sub(arm::x30, arm::x22, arm::x22, arm::Shift::ASR, 21))

    code_block.protect();

    return 0;
}