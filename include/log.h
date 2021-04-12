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

#include <iostream>
#include <string>

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if defined(CDP_LOG_ERROR)
#define logerror(...)                                                      \
    do {                                                                   \
        std::cout << "[ERROR: ";                                           \
        std::cout << std::string(__FILENAME__) << ":" + __LINE__ << "]  "; \
        std::cout << __VA_ARGS__ << "\n";                                  \
    } while (0)
#else
#define logerr(...) \
    {}
#define logerr_f(...) \
    {}
#endif

#if defined(CDP_LOG_WARN)
#define logwarn(...)                                                        \
    do {                                                                    \
        std::cout << "[WARN: ";                                             \
        std::cout << std::string(__FILENAME__) << ":" << __LINE__ << "]  "; \
        std::cout << std::string(__VA_ARGS__) << "\n";                      \
    } while (0)
#else
#define logwarn(...) \
    {}
#define logwarn_f(...) \
    {}
#endif

#if defined(CDP_LOG_INFO)
#define loginfo(...)                                     \
    do {                                                 \
        std::cout << "[INFO: ";                          \
        std::cout << std::string(__FILENAME__) << "]  "; \
        std::cout << std::string(__VA_ARGS__) << "\n";   \
    } while (0)
#else
#define loginfo(...) \
    {}
#define loginfo_f(...) \
    {}
#endif

#if defined(CDP_LOG_DEBUG)
#define logdebug(...)                                    \
    do {                                                 \
        std::cout << "[DEBUG: ";                         \
        std::cout << std::string(__FILENAME__) << "]  "; \
        std::cout << std::string(__VA_ARGS__) << "\n";   \
    } while (0)
#else
#define logdbg(...) \
    {}
#define logdbg_f(...) \
    {}
#endif
#endif