#ifndef TIMECOLLECTOR_H
#define TIMECOLLECTOR_H

#include <iostream>
#include <vector>
#include <mutex>

#ifndef TIME_PROFILE
#define TIME_PROFILE 0
#endif

#if TIME_PROFILE >= 1

#define START_COLLECT_TIME(name, counter)             \
    TimeCollectorEntry __timeCollector##name(counter);

#define PAUSE_COLLECT_TIME(name)\
    __timeCollector##name.Pause();

#define CONTINUE_COLLECT_TIME(name)\
    __timeCollector##name.Continue();

#define STOP_COLLECT_TIME(name)\
    __timeCollector##name.Stop();

#else

#define START_COLLECT_TIME(name, counter);
#define PAUSE_COLLECT_TIME(name);
#define CONTINUE_COLLECT_TIME(name);
#define STOP_COLLECT_TIME(name);

#endif

enum class Counters : int
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
    CountersCount
};

class TimeCollectorEntry
{

public:
    
    TimeCollectorEntry(Counters counter);
    ~TimeCollectorEntry();

    TimeCollectorEntry(TimeCollectorEntry& entry) = delete;
    TimeCollectorEntry& operator=(const TimeCollectorEntry& entry) = delete;

    void Pause();
    void Continue();
    void Stop();

    Counters getCounter() const {
        return _counter;
    }
    
    ulong getStart() const {
        return _start;
    }
    
    ulong getLength() const {
        return _length;
    }

private:
    
    Counters _counter;
    ulong _start;
    ulong _length;
    bool _collected;
};

class TimeBaseEntry
{
    
public:
    
    TimeBaseEntry(Counters counter, ulong threadId, ulong start, ulong length):
        _counter(counter),
        _threadId(threadId),
        _start(start),
        _length(length)
    {}

    Counters getCounter() const {
        return _counter;
    }
    
    ulong getThreadId() const {
        return _threadId;
    }
    
    ulong getStart() const {
        return _start;
    }
    
    ulong getLength() const {
        return _length;
    }

private:
    
    Counters _counter;
    ulong _start;
    ulong _length;
    ulong _threadId;
    
};

class TimeCollector
{

public:
    
    static void Initialize();
    static void AddToTimeCollector(const TimeCollectorEntry& entry);
    static void PrintInfo(std::ostream& stream);

    static ulong GetThreadId();
    static ulong GetTickCount();

    static void ThreadInitialize();
    static void ThreadFinalize();

};

#endif // TIMECOLLECTOR_H
