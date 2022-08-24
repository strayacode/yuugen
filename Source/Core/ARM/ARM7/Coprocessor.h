#pragma once

#include "Core/ARM/CoprocessorBase.h"

class System;

class ARM7Coprocessor : public CoprocessorBase {
public:
    ARM7Coprocessor(System& system);

private:
    System& system;
};