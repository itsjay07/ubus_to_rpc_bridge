#ifndef LOGGING_H
#define LOGGING_H
#include <stdio.h>
#include <time.h>
#define LOG_COLOR_RED     "\x1b[31m"
#define LOG_COLOR_GREEN   "\x1b[32m"
#define LOG_COLOR_YELLOW  "\x1b[33m"
#define LOG_COLOR_BLUE    "\x1b[34m"
#define LOG_COLOR_RESET   "\x1b[0m"
static inline void log_timestamp() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
    fflush(stdout);
}
#define log_info(fmt, ...) do { log_timestamp(); printf(LOG_COLOR_GREEN "INFO" LOG_COLOR_RESET ": " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#define log_debug(fmt, ...) do { log_timestamp(); printf(LOG_COLOR_BLUE "DEBUG" LOG_COLOR_RESET ": " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#define log_warn(fmt, ...) do { log_timestamp(); printf(LOG_COLOR_YELLOW "WARN" LOG_COLOR_RESET ": " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#define log_error(fmt, ...) do { log_timestamp(); printf(LOG_COLOR_RED "ERROR" LOG_COLOR_RESET ": " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#endif
