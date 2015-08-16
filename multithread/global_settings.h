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
#define DEBUG_BLOCK_START\
    {\
       std::unique_lock<std::mutex> debug_lock(getDebugStreamLock(), std::defer_lock);\
       debug_lock.lock();
#define DEBUG_BLOCK_END\
    }
#else
#define DEBUG_INFO(args);
#define DEBUG_BLOCK_START\
    if(false){
#define DEBUG_BLOCK_END\
    }
#endif

#ifdef TIME_PROFILE
#define COLLECT_TIME(counter)\
    TimeCollectorEntry __timeCollectorEntry(static_cast<int>(counter));
#else
#define COLLECT_TIME(counter);
#endif

#ifdef MULTITHREAD
#define TAKE_LOCK(mutex_name)\
    std::lock_guard<std::mutex> lock(mutex_name);
#else
#define TAKE_LOCK(mutex);
#endif

#endif // GLOBAL_SETTINGS_H
