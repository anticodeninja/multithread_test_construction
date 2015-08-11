#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <ostream>

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

std::ostream& getDebugStream();

#ifdef DEBUG_MODE
#define DEBUG_INFO(args)\
    getDebugStream() << args << std::endl;
#else
#define DEBUG_INFO(args);
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
