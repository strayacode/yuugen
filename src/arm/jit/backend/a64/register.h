#pragma once

#include "common/types.h"

namespace arm {

struct Reg {
    constexpr explicit Reg(u32 id, u32 bits) : id(id), bits(bits) {}

    u32 id;
    u32 bits;
};

struct WReg : public Reg {
    constexpr explicit WReg() : Reg(-1, 32) {}
    constexpr explicit WReg(u32 id) : Reg(id, 32) {}
};

struct XReg : public Reg {
    constexpr explicit XReg() : Reg(-1, 32) {}
    constexpr explicit XReg(u32 id) : Reg(id, 64) {}
};

enum SystemReg {
    NZCV = 0xda10,
};

inline constexpr XReg x0{0};
inline constexpr XReg x1{1};
inline constexpr XReg x2{2};
inline constexpr XReg x3{3};
inline constexpr XReg x4{4};
inline constexpr XReg x5{5};
inline constexpr XReg x6{6};
inline constexpr XReg x7{7};
inline constexpr XReg x8{8};
inline constexpr XReg x9{9};
inline constexpr XReg x10{10};
inline constexpr XReg x11{11};
inline constexpr XReg x12{12};
inline constexpr XReg x13{13};
inline constexpr XReg x14{14};
inline constexpr XReg x15{15};
inline constexpr XReg x16{16};
inline constexpr XReg x17{17};
inline constexpr XReg x18{18};
inline constexpr XReg x19{19};
inline constexpr XReg x20{20};
inline constexpr XReg x21{21};
inline constexpr XReg x22{22};
inline constexpr XReg x23{23};
inline constexpr XReg x24{24};
inline constexpr XReg x25{25};
inline constexpr XReg x26{26};
inline constexpr XReg x27{27};
inline constexpr XReg x28{28};
inline constexpr XReg x29{29};
inline constexpr XReg x30{30};
inline constexpr XReg sp{31};
inline constexpr XReg xzr{31};

inline constexpr WReg w0{0};
inline constexpr WReg w1{1};
inline constexpr WReg w2{2};
inline constexpr WReg w3{3};
inline constexpr WReg w4{4};
inline constexpr WReg w5{5};
inline constexpr WReg w6{6};
inline constexpr WReg w7{7};
inline constexpr WReg w8{8};
inline constexpr WReg w9{9};
inline constexpr WReg w10{10};
inline constexpr WReg w11{11};
inline constexpr WReg w12{12};
inline constexpr WReg w13{13};
inline constexpr WReg w14{14};
inline constexpr WReg w15{15};
inline constexpr WReg w16{16};
inline constexpr WReg w17{17};
inline constexpr WReg w18{18};
inline constexpr WReg w19{19};
inline constexpr WReg w20{20};
inline constexpr WReg w21{21};
inline constexpr WReg w22{22};
inline constexpr WReg w23{23};
inline constexpr WReg w24{24};
inline constexpr WReg w25{25};
inline constexpr WReg w26{26};
inline constexpr WReg w27{27};
inline constexpr WReg w28{28};
inline constexpr WReg w29{29};
inline constexpr WReg w30{30};
inline constexpr WReg wzr{31};

} // namespace arm;