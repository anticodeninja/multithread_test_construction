#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#define DEBUG_MODE 1
#define TIME_PROFILE 1

enum TimeCollectorCategory
{
    All,
    ReadingInput,
    PreparingInput,
    PlanBuilding,
    QHandling,
    RMerging,
    WritingOutput,
    Threading,
    CrossThreading,
    TimeCollectorCount
};

#ifdef DEBUG_MODE
#define DEBUG_INFO(args)\
    std::cout << args << std::endl;
#else
#define DEBUG_INFO(args);
#endif

#ifdef TIME_PROFILE
#define COLLECT_TIME(counter)\
    TimeCollectorEntry __timeCollectorEntry_##counter(counter);
#else
#define COLLECT_TIME(counter);
#endif

#endif // GLOBAL_SETTINGS_H
