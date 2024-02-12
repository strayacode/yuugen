#pragma once

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <string_view>
#include "common/string.h"

namespace common {

enum class LogLevel {
    Info,
    Debug,
    Warn,
    Error,
    Todo,
    Log,
};

#define RED "\x1B[1;31m"
#define GREEN "\x1B[0;32m"
#define YELLOW "\x1B[1;33m"
#define BLUE "\x1B[0;34m"
#define WHITE "\x1B[0;37m"
#define GREY "\x1B[0;37m"
#define RESET "\x1B[0m"
#define BOLD "\x1B[0m"

inline const char* get_colour_from_level(LogLevel log_level) {
    switch (log_level) {
    case LogLevel::Info:
        return GREY;
    case LogLevel::Debug:
        return GREEN;
    case LogLevel::Warn:
        return YELLOW;
    case LogLevel::Error:
        return RED;
    case LogLevel::Todo:
        return YELLOW;
    case LogLevel::Log:
        return RESET;    
    }
}

constexpr const char* trim_source_path(std::string_view source) {
    // only include everything beyond /src
    const std::string_view match = "/src";
    const auto index = source.rfind(match) == source.npos ? 0 : source.rfind(match) + match.size();
    return source.data() + index;
}

template <typename... Args>
void log_impl(LogLevel log_level, const char* source, int line, const char* function, const char* pattern, Args... args) {
    printf("%s\n", common::format(pattern, std::forward<Args>(args)...).c_str());
    // printf("%s%s:%d @ %s: %s\n", get_colour_from_level(log_level), trim_source_path(source), line, function, common::format(pattern, std::forward<Args>(args)...).c_str());

    if (log_level == LogLevel::Error || log_level == LogLevel::Todo) {
        std::exit(0);
    }
}

} // namespace common

#define LOG_INFO(pattern, ...) common::log_impl(common::LogLevel::Info, __FILE__, __LINE__, __FUNCTION__, pattern, ##__VA_ARGS__);
#define LOG_DEBUG(pattern, ...) common::log_impl(common::LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__, pattern, ##__VA_ARGS__);
#define LOG_WARN(pattern, ...) common::log_impl(common::LogLevel::Warn, __FILE__, __LINE__, __FUNCTION__, pattern, ##__VA_ARGS__);
#define LOG_ERROR(pattern, ...) common::log_impl(common::LogLevel::Error, __FILE__, __LINE__, __FUNCTION__, pattern, ##__VA_ARGS__);
#define LOG_TODO(pattern, ...) common::log_impl(common::LogLevel::Todo, __FILE__, __LINE__, __FUNCTION__, pattern, ##__VA_ARGS__);