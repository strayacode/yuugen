#pragma once

#include "Core/ARM/CoprocessorBase.h"

class System;

class ARM9Coprocessor : public CoprocessorBase {
public:
    ARM9Coprocessor(System& system);

private:
    System& system;
};