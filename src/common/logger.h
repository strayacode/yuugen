#pragma once

#include <cstdio>
#include "common/string.h"

class Logger {
public:
    template <typename... Args>
    void debug(const char* pattern, Args... args) {
        std::printf("[DEBUG] %s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void info(const char* pattern, Args... args) {
        std::printf("[INFO] %s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void warn(const char* pattern, Args... args) {
        std::printf("[WARN] %s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

    template <typename... Args>
    void error(const char* pattern, Args... args) {
        std::printf("[ERROR] %s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    }

private:
};

extern Logger logger;


// #define log_debug(message, ...) fprintf(stdout, GREEN message "\n" RESET, ##__VA_ARGS__);
// #define log_warn(message, ...) fprintf(stdout, YELLOW message "\n" RESET, ##__VA_ARGS__);
// #define log_fatal(message, ...) fprintf(stderr, RED "fatal at " RESET "%s:%d " RED message "\n" RESET, __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE);

// #define todo() fprintf(stderr, RED "todo at %s:%s:%d" RESET " " RED "\n" RESET, __FILE__, __func__, __LINE__); exit(EXIT_FAILURE);