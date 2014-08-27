#ifndef TIMECOLLECTOR_H
#define TIMECOLLECTOR_H

#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point time_point;
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

private:
    int _state;
    time_point _startTime;
    time_point _endTime;

    bool _collected;
};

class TimeBaseEntry
{
public:
    TimeBaseEntry(int state, time_point start, time_point end) {
        _state = state;
        _startTime = start;
        _endTime = end;
    }

    int getState() const { return _state; }
    const time_point& getStartTime() const { return _startTime; }
    const time_point& getEndTime() const { return _endTime; }

private:
    int _state;
    time_point _startTime;
    time_point _endTime;
};

class TimeCollector
{

public:
    static void Initialize(int reserved);
    static void AddToTimeCollector(const TimeCollectorEntry& entry);
    static void PrintInfo(std::ostream& stream);

private:
    static time_point _initializeTime;
    static std::vector<TimeBaseEntry> _timeCollectors;
    static std::mutex _mutex;

};

#endif // TIMECOLLECTOR_H
