#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3

#define LOG_LEVEL LOG_LEVEL_INFO

std::ofstream *getLogger();

#define LOG_WITH_LEVEL(level, logger, fmt, ...)                                             \
    do                                                                                      \
    {                                                                                       \
        char s[1024];                                                                       \
        sprintf(s, fmt, __VA_ARGS__);                                                       \
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());    \
        auto tm = std::string(std::ctime(&t)).substr(0, 24);                                \
        (*getLogger()) << tm << " - [" << logger << "] " << level << ' ' << s << std::endl; \
    } while (0)

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(logger, fmt, ...) LOG_WITH_LEVEL("DEBUG", logger, fmt, __VA_ARGS__)
#else
#define LOG_DEBUG(logger, fmt, ...)
#endif
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(logger, fmt, ...) LOG_WITH_LEVEL("INFO", logger, fmt, __VA_ARGS__)
#else
#define LOG_INFO(logger, fmt, ...)
#endif
#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(logger, fmt, ...) LOG_WITH_LEVEL("WARNING", logger, fmt, __VA_ARGS__)
#else
#define LOG_WARNING(logger, fmt, ...)
#endif
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(logger, fmt, ...) LOG_WITH_LEVEL("ERROR", logger, fmt, __VA_ARGS__)
#else
#define LOG_ERROR(logger, fmt, ...)
#endif