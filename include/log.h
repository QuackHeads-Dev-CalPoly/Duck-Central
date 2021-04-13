#ifndef LOG_H
#define LOG_H

#define CDP_DEBUG

// always log errors and warns
#define CDP_LOG_ERROR
#define CDP_LOG_WARN

// info enables errors, warns, info
#ifdef CDP_INFO
#define CDP_LOG_INFO
#endif

// debug enables errors, warns, info, debug
#ifdef CDP_DEBUG
#define CDP_LOG_INFO
#define CDP_LOG_DEBUG
#endif

#include <stdio.h>
#include <string.h>

#define __FILENAME__ strrchr("/" __FILE__, '/') + 1

#if defined(CDP_LOG_ERROR)
#define logerror(f_, ...)                          \
    do {                                           \
        printf("[ERROR: ");                        \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);               \
    } while (0)
#else
#define logerror(...) \
    {}
#endif

#if defined(CDP_LOG_WARN)
#define logwarn(f_, ...)                           \
    do {                                           \
        printf("[WARN: ");                         \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);               \
    } while (0)
#else
#define logwarn(...) \
    {}
#endif

#if defined(CDP_LOG_INFO)
#define loginfo(f_, ...)                           \
    do {                                           \
        printf("[INFO: ");                         \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);               \
    } while (0)
#else
#define loginfo(...) \
    {}
#endif

#if defined(CDP_LOG_DEBUG)
#define logdebug(f_, ...)                          \
    do {                                           \
        printf("[DEBUG: ");                        \
        printf("%s:%d] ", __FILENAME__, __LINE__); \
        printf((f_), ##__VA_ARGS__);               \
    } while (0)
#else
#define logdebug(...) \
    {}
#endif
#endif
