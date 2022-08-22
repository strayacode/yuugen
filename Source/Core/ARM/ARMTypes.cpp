#include "Core/ARM/ARMTypes.h"

std::string get_arch(Arch arch) {
    return arch == Arch::ARMv5 ? "arm9" : "arm7";
}