#pragma once

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include "common/string.h"

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

class Logger {
public:
    template <typename... Args>
    void debug(const char* pattern, Args... args) {
        set_text_colour(TextColour::Green);
        Time time = get_current_time();
        std::printf("[%02d:%02d:%02d] %s\n", time.hour, time.minute, time.second, common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void info(const char* pattern, Args... args) {
        set_text_colour(TextColour::White);
        Time time = get_current_time();
        std::printf("[%02d:%02d:%02d] %s\n", time.hour, time.minute, time.second, common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void warn(const char* pattern, Args... args) {
        set_text_colour(TextColour::Yellow);
        Time time = get_current_time();
        std::printf("[%02d:%02d:%02d] %s\n", time.hour, time.minute, time.second, common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void error(const char* pattern, Args... args) {
        set_text_colour(TextColour::Red);
        Time time = get_current_time();
        std::printf("[%02d:%02d:%02d] %s\n", time.hour, time.minute, time.second, common::format(pattern, std::forward<Args>(args)...).c_str());
        std::exit(0);
    }

    template <typename... Args>
    void todo(const char* pattern, Args... args) {
        set_text_colour(TextColour::Red);
        Time time = get_current_time();
        std::printf("[%02d:%02d:%02d] TODO: %s\n", time.hour, time.minute, time.second, common::format(pattern, std::forward<Args>(args)...).c_str());
        std::exit(0);
    }

    template <typename... Args>
    void print(const char* pattern, Args... args) {
        reset_colour();
        std::printf("%s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void log(const char* pattern, Args... args) {
        std::fprintf(fp, "%s", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

private:
    struct Time {
        int hour;
        int minute;
        int second;
    };

    enum class TextColour {
        Green,
        Yellow,
        Red,
        White,
    };

    Time get_current_time() {
        std::time_t time = std::time(nullptr);
        std::tm* local_time = std::localtime(&time);
        int hour = local_time->tm_hour;
        int minute = local_time->tm_min;
        int second = local_time->tm_sec;
        return {hour, minute, second};
    }

    void set_text_colour(TextColour colour) {
        switch (colour) {
        case TextColour::Red:
            std::printf("%s", RED);
            break;
        case TextColour::Yellow:
            std::printf("%s", YELLOW);
            break;
        case TextColour::Green:
            std::printf("%s", GREEN);
            break;
        case TextColour::White:
            std::printf("%s", WHITE);
        }
    }

    void reset_colour() {
        std::printf("%s", RESET);
    }

    FILE* fp = fopen("yuugen.log", "w");
};

extern Logger logger;