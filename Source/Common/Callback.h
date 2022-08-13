#pragma once

#include <utility>
#include <memory>
#include <type_traits>
#include <cstdarg>
#include "Common/Types.h"

namespace Common {

template <typename Func>
class Callback;

template <typename Return, typename... Args>
class Callback<Return(Args...)> {
public:
    Callback() = default;

    template <typename Func>
    Callback(Func&& func) {
        ptr = (void*)std::addressof(func);

        fn = [](void* ptr, Args... args) -> Return {
            return (*reinterpret_cast<Func*>(ptr))(std::forward<Args>(args)...);
        };
    }

    Return operator()(Args... args) {
        // printf("")
        std::va_list list;
        printf("arg %08x\n", va_arg(list, u8));
        printf("arg %08x\n", va_arg(list, u8));
        printf("arg %08x\n", va_arg(list, u8));
        printf("arg %08x\n", va_arg(list, u8));
        if (std::is_void_v<Return>) {
            printf("bad\n");
        }

        va_end(list);

        return fn(ptr, args...);
    }
    
private:
    void* ptr = nullptr;
    Return (*fn)(void*, Args...) = nullptr;
};

}