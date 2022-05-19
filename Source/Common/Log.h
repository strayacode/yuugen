#include <stdio.h>
#include <stdlib.h>

// define the colour escape codes
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"


#define RESET "\x1B[0m"

#define log_debug(message, ...) fprintf(stdout, GREEN message "\n" RESET, ##__VA_ARGS__);
#define log_warn(message, ...) fprintf(stdout, YELLOW message "\n" RESET, ##__VA_ARGS__);
#define log_fatal(message, ...) fprintf(stderr, RED "fatal at " RESET "%s:%d " RED message "\n" RESET, __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE);