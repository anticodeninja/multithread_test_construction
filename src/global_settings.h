#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <ostream>
#include <mutex>

std::mutex& getDebugStreamLock();
std::ostream& getDebugStream();

#ifdef DEBUG_MODE
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
#define DEBUG_INFO(args);
#define DEBUG_BLOCK(commands);
#endif

#ifdef MULTITHREAD_DIVIDE2
#define MULTITHREAD
#endif

#ifdef MULTITHREAD_MASTERWORKER
#define MULTITHREAD
#endif

#endif // GLOBAL_SETTINGS_H
