#pragma once

class Logger {
public:
    void debug() {

    }

    void info() {

    }

    void warn() {

    }

    void error() {

    }

private:
};

extern Logger logger;


// #define log_debug(message, ...) fprintf(stdout, GREEN message "\n" RESET, ##__VA_ARGS__);
// #define log_warn(message, ...) fprintf(stdout, YELLOW message "\n" RESET, ##__VA_ARGS__);
// #define log_fatal(message, ...) fprintf(stderr, RED "fatal at " RESET "%s:%d " RED message "\n" RESET, __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE);

// #define todo() fprintf(stderr, RED "todo at %s:%s:%d" RESET " " RED "\n" RESET, __FILE__, __func__, __LINE__); exit(EXIT_FAILURE);