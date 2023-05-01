#pragma once

namespace core::nds {

class Input {
public:
    u16 read_extkeyin();
private:
    u16 extkeyin;
};

} // namespace core::nds