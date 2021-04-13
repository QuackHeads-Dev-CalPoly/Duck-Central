#ifndef LOG_H
#define LOG_H

#define CDP_DEBUG

#ifdef CDP_DEBUG
#define CDP_LOG_ERROR
#define CDP_LOG_INFO
#define CDP_LOG_DEBUG
#define CDP_LOG_WARN
#endif

#include <stdio.h>
#include <string.h>

#define __FILENAME__ strrchr("/" __FILE__, '/') + 1

#if defined(CDP_LOG_ERROR)
#define logerror(f_, ...)                          \
    do {                                           \
        printf("[ERROR: ");                        \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);                 \
    } while (0)
#else
#define logerr(...) \
    {}
#define logerr_f(...) \
    {}
#endif

#if defined(CDP_LOG_WARN)
#define logwarn(f_, ...)                           \
    do {                                           \
        printf("[WARN: ");                         \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);                 \
    } while (0)
#else
#define logwarn(...) \
    {}
#define logwarn_f(...) \
    {}
#endif

#if defined(CDP_LOG_INFO)
#define loginfo(f_, ...)                               \
    do {                                           \
        printf("[INFO: ");                         \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);                 \
    } while (0)
#else
#define loginfo(...) \
    {}
#define loginfo_f(...) \
    {}
#endif

#if defined(CDP_LOG_DEBUG)
#define logdebug(f_, ...)                          \
    do {                                           \
        printf("[DEBUG: ");                        \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);                 \
    } while (0)
#else
#define logdbg(...) \
    {}
#define logdbg_f(...) \
    {}
#endif
#endif
