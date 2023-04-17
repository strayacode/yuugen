#pragma once

namespace core {

class System {
public:
    virtual void run_frame() = 0;
    virtual void reset() = 0;

private:
};

} // namespace core