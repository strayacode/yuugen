#include <stdio.h>
#include <stdlib.h>

#define RED   "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW   "\x1B[33m"
// #define BLU   "\x1B[34m"
// #define MAG   "\x1B[35m"
// #define CYN   "\x1B[36m"
// #define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


#define log_fatal(message, ...) fprintf(stderr, RED "[FATAL] at " RESET "%s:%d ", __FILE__, __LINE__); fprintf(stderr, RED message "\n" RESET, ##__VA_ARGS__); exit(EXIT_FAILURE);

#define log_warn(message, ...) fprintf(stdout, YELLOW "[WARN] " message "\n" RESET, ##__VA_ARGS__);

#define log_debug(message, ...) fprintf(stdout, GREEN "[DEBUG] " message "\n" RESET, ##__VA_ARGS__);