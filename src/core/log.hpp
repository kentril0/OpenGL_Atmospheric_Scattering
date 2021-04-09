/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file log.hpp
 * @brief Logging and debugging functions
 *********************************************************/

#pragma once

//#include <iostream>
#include <iomanip>

// Logging levels from highest to lowest
#define LEVEL_OK        0
#define LEVEL_INFO      1
#define LEVEL_WARNING   2
#define LEVEL_ERROR     3

/* Logging functions:
 * LOG_OK("Eg" << smth << "done")
 * LOG_INFO()
 * LOG_WARN()
 * LOG_ERR()
*/

// Text colors of logging outputs (for console)
#define COLOR_NORMAL    "\u001b[0m"
#define COLOR_RED       "\u001b[31m"
#define COLOR_YELLOW    "\u001b[33m"
#define COLOR_GREEN     "\u001b[32m"


/**
 * @brief Returns current system time
 */
auto inline curtime()
{
    time_t t = time(0);
    return std::put_time((localtime(&t)), "%T");
}

// ----------------------------------------
// ASSERT
#ifdef ENABLE_ASSERTS
    #define massert(cond, msg) if (!(cond)) { throw msg; }
#else
    #define massert(cond, msg) {}
#endif

// ----------------------------------------
///< LOG_OK(text)
#if LOG_LEVEL <= LEVEL_OK
    #define LOG_OK(x) do {                                      \
        std::cerr << COLOR_GREEN << '[' << curtime() << "]: "   \
                  << x << COLOR_NORMAL << std::endl; } while(0)
#else
    #define LOG_OK(x) do { } while(0)
#endif

// ----------------------------------------
///< LOG_INFO(text)
#if LOG_LEVEL <= LEVEL_INFO
    #define LOG_INFO(x) do {                                                 \
        std::cerr << '[' << curtime() << "]: " << x << std::endl; } while(0)
#else
    #define LOG_INFO(x) do { } while(0)
#endif

// ----------------------------------------
///< LOG_WARN(text)
#if LOG_LEVEL <= LEVEL_WARNING
    #define LOG_WARN(x) do {                                            \
        std::cerr << COLOR_YELLOW << '[' << curtime() << "]: WARNING: " \
                  << x << COLOR_NORMAL << std::endl; } while(0)
#else
    #define LOG_WARN(x) do { } while(0)
#endif

// ----------------------------------------
///< LOG_ERR(text)
#if LOG_LEVEL <= LEVEL_ERROR
    #define LOG_ERR(x) do {                                         \
        std::cerr << COLOR_RED << '[' << curtime() << "]: ERROR: "  \
                  << x << COLOR_NORMAL << std::endl; } while(0)
#else
    #define LOG_ERR(x) do { } while(0)
#endif

#ifdef DEBUG
    #define DERR(x) do { \
      std::cerr << std::string(__FILE__).substr(1+std::string(__FILE__).rfind('/')) \
                << ':' << __LINE__ << ": " << x << std::endl;   \
    } while(0)
#else
    #define DERR(x) do { } while(0)
#endif

