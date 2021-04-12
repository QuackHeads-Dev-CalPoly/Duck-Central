#ifndef LOG_H
#define LOG_H

#define CDP_DEBUG

#ifdef CDP_DEBUG
#define CDP_LOG_ERROR
#define CDP_LOG_INFO
#define CDP_LOG_DEBUG
#define CDP_LOG_WARN
#endif

#include <string.h>
#include <stdio.h>

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if defined(CDP_LOG_ERROR)
#define logerror(...)                               \
    do {                                            \
        printf("[ERROR: ");                         \
        printf(__FILENAME__, ":", __LINE__, "]  "); \
        printf(__VA_ARGS__, "\n");                  \
    } while (0)
#else
#define logerr(...) \
    {}
#define logerr_f(...) \
    {}
#endif

#if defined(CDP_LOG_WARN)
#define logwarn(...)                                \
    do {                                            \
        printf("[WARN: ");                          \
        printf(__FILENAME__, ":", __LINE__, "]  "); \
        printf(__VA_ARGS__, "\n");                  \
    } while (0)
#else
#define logwarn(...) \
    {}
#define logwarn_f(...) \
    {}
#endif

#if defined(CDP_LOG_INFO)
#define loginfo(...)                 \
    do {                             \
        printf("[INFO: ");           \
        printf(__FILENAME__, "]  "); \
        printf(__VA_ARGS__, "\n");   \
    } while (0)
#else
#define loginfo(...) \
    {}
#define loginfo_f(...) \
    {}
#endif

#if defined(CDP_LOG_DEBUG)
#define logdebug(...)                \
    do {                             \
        printf("[DEBUG: ");          \
        printf(__FILENAME__, "]  "); \
        printf(__VA_ARGS__, "\n");   \
    } while (0)
#else
#define logdbg(...) \
    {}
#define logdbg_f(...) \
    {}
#endif
#endif
