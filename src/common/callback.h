#pragma once

#include <utility>
#include <memory>
#include <type_traits>
#include <cassert>
#include "common/types.h"
#include "common/logger.h"

namespace common {

template <typename Func, size_t N = 16>
class Callback;

template <typename ReturnType, typename... Args, size_t N>
class Callback<ReturnType(Args...), N> {
public:
    Callback() = default;

    template <typename Func>
    Callback(Func func) {
        static_assert(sizeof(Func) <= N, "Not enough space for lambda state");

        new (state) Func(static_cast<Func&&>(func));

        fn = [](void* ptr, Args... args) -> ReturnType {
            return (*reinterpret_cast<Func*>(ptr))(std::forward<Args>(args)...);
        };
    }

    ReturnType operator()(Args... args) {
        return fn(state, std::forward<Args>(args)...);
    }
    
private:
    ReturnType (*fn)(void*, Args...) = nullptr;

    // store all lambda state
    char state[N] = {0};
};

} // namespace common