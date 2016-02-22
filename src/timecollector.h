#ifndef TIMECOLLECTOR_H
#define TIMECOLLECTOR_H

#include <iostream>
#include <chrono>
#include <thread>

#ifndef TIME_PROFILE
#define TIME_PROFILE 0
#endif

#if TIME_PROFILE >= 2
#include <vector>
#include <mutex>
#else
#include <atomic>
#endif

#if TIME_PROFILE >= 1

#define START_COLLECT_TIME(name, counter)\
    TimeCollectorEntry name(counter);

#define STOP_COLLECT_TIME(name)\
    name.Stop();

#else

#define START_COLLECT_TIME(name, counter);
#define STOP_COLLECT_TIME(name);

#endif

typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::nanoseconds time_duration;

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

    void Stop();

    Counters getCounter() const { return _counter; }
    const time_point& getStartTime() const { return _startTime; }
    const time_point& getEndTime() const { return _endTime; }

#if TIME_PROFILE >= 2
    const std::thread::id& getThreadId() const { return _threadId; }
#endif

private:
    
    Counters _counter;
    time_point _startTime;
    time_point _endTime;

#if TIME_PROFILE >= 2
    std::thread::id _threadId;
#endif

    bool _collected;
};

#if TIME_PROFILE >= 2

class TimeBaseEntry
{
    
public:
    
    TimeBaseEntry(Counters counter, std::thread::id threadId, time_point start, time_point end):
        _counter(counter),
        _threadId(threadId),
        _startTime(start),
        _endTime(end)
    {}

    Counters getCounter() const { return _counter; }
    const time_point& getStartTime() const { return _startTime; }
    const time_point& getEndTime() const { return _endTime; }
    const std::thread::id& getThreadId() const { return _threadId; }

private:
    
    Counters _counter;
    time_point _startTime;
    time_point _endTime;
    std::thread::id _threadId;
    
};

#endif

class TimeCollector
{

public:
    
    static void Initialize();
    static void AddToTimeCollector(const TimeCollectorEntry& entry);
    static void PrintInfo(std::ostream& stream);

private:
    static time_point _initializeTime;

#if TIME_PROFILE >= 2
    static std::vector<TimeBaseEntry> _timeEntries;
    static std::mutex _mutex;
#else
    static std::atomic<ulong> _totalTime[];
#endif

};

#endif // TIMECOLLECTOR_H
