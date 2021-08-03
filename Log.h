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

#define LOG_WITH_LEVEL(level, logger, fmt, ...)                                                       \
    do                                                                                                \
    {                                                                                                 \
        char _logstr[1024];                                                                           \
        sprintf(_logstr, fmt, __VA_ARGS__);                                                           \
        auto _logt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());          \
        auto _logtm = std::string(std::ctime(&_logt)).substr(0, 24);                                  \
        (*getLogger()) << _logtm << " - [" << logger << "] " << level << ' ' << _logstr << std::endl; \
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