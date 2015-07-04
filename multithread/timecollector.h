#ifndef TIMECOLLECTOR_H
#define TIMECOLLECTOR_H

#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include <map>
#include <thread>

typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::nanoseconds time_duration;

class TimeCollectorEntry
{

public:
    TimeCollectorEntry(int state);
    ~TimeCollectorEntry();

    TimeCollectorEntry(TimeCollectorEntry& entry) = delete;
    TimeCollectorEntry& operator=(const TimeCollectorEntry& entry) = delete;

    void Stop();

    int getState() const { return _state; }
    const time_point& getStartTime() const { return _startTime; }
    const time_point& getEndTime() const { return _endTime; }
    const std::thread::id& getThreadId() const { return _threadId; }

private:
    int _state;
    time_point _startTime;
    time_point _endTime;
    std::thread::id _threadId;

    bool _collected;
};

class TimeBaseEntry
{
public:
    TimeBaseEntry(int state, std::thread::id threadId, time_point start, time_point end):
        _state(state),
        _threadId(threadId),
        _startTime(start),
        _endTime(end)
    {}

    int getState() const { return _state; }
    const time_point& getStartTime() const { return _startTime; }
    const time_point& getEndTime() const { return _endTime; }
    const std::thread::id& getThreadId() const { return _threadId; }

private:
    int _state;
    time_point _startTime;
    time_point _endTime;
    std::thread::id _threadId;
};

class TimeCollector
{

public:
    static void Initialize(int reserved);
    static void AddToTimeCollector(const TimeCollectorEntry& entry);
    static void PrintInfo(std::ostream& stream, std::map<int, std::string> &states);

private:
    static time_point _initializeTime;
    static std::vector<TimeBaseEntry> _timeCollectors;
    static std::mutex _mutex;

};

#endif // TIMECOLLECTOR_H
