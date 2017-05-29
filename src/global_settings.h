#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <iostream>
#include <iomanip>
#include <limits>
#ifdef MULTITHREAD
#include <mutex>
#endif

typedef uint32_t feature_t;
typedef  uint8_t test_feature_t;
typedef uint32_t feature_size_t;
typedef uint32_t set_size_t;
typedef uint64_t calc_hash_t;
const int calc_hash_bits = std::numeric_limits<calc_hash_t>::digits;

#if MULTITHREAD
#define IF_MULTITHREAD(command) command
#else
#define IF_MULTITHREAD(command);
#endif

#ifdef DEBUG_MODE
std::ostream& getDebugStream();

#ifdef MULTITHREAD
std::mutex& getDebugStreamLock();

#define INIT_DEBUG_OUTPUT()\
    std::ostream& getDebugStream() { return std::cout; }\
    std::mutex debugLock;\
    std::mutex& getDebugStreamLock() { return debugLock; };
#define DEBUG_INFO(args)\
    {\
       std::unique_lock<std::mutex> debug_lock(getDebugStreamLock(), std::defer_lock);\
       debug_lock.lock();\
       getDebugStream() << args << std::endl;\
    }
#define DEBUG_BLOCK(commands)\
    {\
       std::unique_lock<std::mutex> debug_lock(getDebugStreamLock(), std::defer_lock);\
       debug_lock.lock();\
       commands\
    }
#else
#define INIT_DEBUG_OUTPUT()\
    std::ostream& getDebugStream() { return std::cout; };
#define DEBUG_INFO(args) getDebugStream() << args << std::endl;
#define DEBUG_BLOCK(commands) commands
#endif

#else
#define DEBUG_INFO(args);
#define DEBUG_BLOCK(commands);
#define INIT_DEBUG_OUTPUT();
#endif

#endif // GLOBAL_SETTINGS_H
