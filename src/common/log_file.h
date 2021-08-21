#pragma once

#include <stdarg.h>
#include <stdio.h>
#include "log.h"

struct LogFile {
    LogFile(const char *path) {
        fp = fopen(path, "w");

        if (fp == NULL) {
            log_fatal("[LogFile] Path %s doesn't exist yet", path);
        }
    }

    ~LogFile() {
        fclose(fp);
    }

    void Log(const char *format, ...) {
        va_list args;

        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);
    }

    FILE* fp;
};