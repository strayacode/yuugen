#pragma once

#include <utility>
#include <memory>
#include "Common/Types.h"

template <typename Func>
class Callback;

template <typename Return, typename... Args>
class Callback<Return(Args...)> {
public:
    Callback() = delete;

    template <typename Func>
    Callback(Func&& func) {
        ptr = (void*)std::addressof(func);

        fn = [](void* ptr, Args... args) -> Return {
            return (*reinterpret_cast<Func*>(ptr))(
                std::forward<Args>(args)...);
        };
    }

    Return operator()(Args... args) {
        return fn(ptr, args...);
    }
    
private:
    void* ptr = nullptr;
    Return (*fn)(void*, Args...) = nullptr;
};