#pragma once

template <typename Return, typename... Args>
class Callback {
public:
    Callback() = delete;

    template <typename Func>
    Callback(Func&& func) {
        printf("pog\n");
        func();
    }

    // Return operator(Args... args) {

    // }
private:
};

// example of how our callback thing should work
// Callback<void()> cb = [&]() {
//     printf("hi\n");
// };