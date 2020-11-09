#pragma once
#include <cpputil/log/log.h>
#include <signal.h>
#include <cstdlib>

#define INFO_SAMPLING_RATE 101
#define INFO_SAMPLING_RATE2 909
#define INFO_SAMPLING_RATE3 1919
#define INFO_SAMPLING_RATE4 10007

#define INFO_S(id, fmt, ...)                       \
    do {                                           \
        if (id % INFO_SAMPLING_RATE == 0) {        \
            INFO(fmt " (sampled)", ##__VA_ARGS__); \
        }                                          \
    } while (0);
#define INFO_S2(id, fmt, ...)                       \
    do {                                            \
        if (id % INFO_SAMPLING_RATE2 == 0) {        \
            INFO(fmt " (sampled2)", ##__VA_ARGS__); \
        }                                           \
    } while (0);
#define INFO_S3(id, fmt, ...)                       \
    do {                                            \
        if (id % INFO_SAMPLING_RATE3 == 0) {        \
            INFO(fmt " (sampled3)", ##__VA_ARGS__); \
        }                                           \
    } while (0);
#define INFO_S4(id, fmt, ...)                       \
    do {                                            \
        if (id % INFO_SAMPLING_RATE4 == 0) {        \
            INFO(fmt " (sampled4)", ##__VA_ARGS__); \
        }                                           \
    } while (0);

#define DEBUG_S(id, fmt, ...)                       \
    do {                                            \
        if (id % INFO_SAMPLING_RATE == 0) {         \
            DEBUG(fmt " (sampled)", ##__VA_ARGS__); \
        }                                           \
    } while (0);

#define SUICIDE()                              \
    do {                                       \
        FATAL("raise SIGKILL to suicide ..."); \
    } while (0 != raise(SIGKILL));             \
    exit(-1);

#define PANIC(fmt, ...)            \
    do {                           \
        FATAL(fmt, ##__VA_ARGS__); \
        SUICIDE();                 \
    } while (0);

// enable large log
#define COLLECT_LOGGER_INFO(fmt, ...) \
    LOG_INTERNAL__(log4cxx::Logger::getRootLogger(), log4cxx::Level::getInfo(), 40960, fmt, ##__VA_ARGS__)
