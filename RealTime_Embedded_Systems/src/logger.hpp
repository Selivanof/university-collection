#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdio.h>
#include <time.h>

// Define log levels
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

// Set default log level
#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO
#endif

// Helper function to get current time as a string. Is thread safe.
inline const char *get_current_time() {
    thread_local char buffer[20];
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(buffer, 20, "%d/%m %H:%M:%S", &tm_info);
    return buffer;
}

// Define LOG_{LEVEL} Macros
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(...)                          \
    printf("[%s] ERROR: ", get_current_time()); \
    printf(__VA_ARGS__)
#else
#define LOG_ERROR(...) /* Do nothing */
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARNING(...)                          \
    printf("[%s] WARNING: ", get_current_time()); \
    printf(__VA_ARGS__)
#else
#define LOG_WARNING(...) /* Do nothing */
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(...)                          \
    printf("[%s] INFO: ", get_current_time()); \
    printf(__VA_ARGS__)
#else
#define LOG_INFO(...) /* Do nothing */
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...)                          \
    printf("[%s] DEBUG: ", get_current_time()); \
    printf(__VA_ARGS__)
#else
#define LOG_DEBUG(...) /* Do nothing */
#endif

#endif  // LOGGER_HPP
