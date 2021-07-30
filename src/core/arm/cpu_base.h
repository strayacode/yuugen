#pragma once

// this is a base class which is used by
// both the arm7 and arm9 and can be used
// for cores such as an interpreter,
// cached interpreter and jit
class CPUBase {
public:
    CPUBase() {};
    virtual ~CPUBase() {};
    // virtual void Reset() = 0;
    // virtual void Run(int cycles) = 0;
};