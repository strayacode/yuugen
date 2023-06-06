#pragma once

#include <stdarg.h>
#include <stdio.h>

#define USE_LOGGING

class LogFile {
public:
    LogFile(const LogFile& log_file) = delete;

    ~LogFile() {
        fclose(fp);
    }

    static LogFile& Get() {
        return instance;
    }

    void Log(const char *format, ...) {
        #ifdef USE_LOGGING

        va_list args;

        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);

        #endif
    }

private:
    LogFile() {};

    FILE* fp = fopen("yuugen-old.log", "w");
    static LogFile instance;
};