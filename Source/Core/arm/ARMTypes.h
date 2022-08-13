#pragma once

#include <string>

enum class Arch {
    ARMv4 = 0,
    ARMv5 = 1,
};

std::string get_arch(Arch arch);