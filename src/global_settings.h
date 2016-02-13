#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <ostream>
#include <mutex>

enum class Timers : int
{
    All,
    ReadingInput,
    PreparingInput,
    CalcR2Matrix,
    SortMatrix,
    CalcR2Indexes,
    PlanBuilding,
    QHandling,
    RMerging,
    WritingOutput,
    Threading,
    CrossThreading,
    TimeCollectorCount
};

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

#ifdef TIME_PROFILE
#define COLLECT_TIME(counter)\
    TimeCollectorEntry __timeCollectorEntry(static_cast<int>(counter));
#else
#define COLLECT_TIME(counter);
#endif

#endif // GLOBAL_SETTINGS_H
