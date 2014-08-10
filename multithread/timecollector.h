#ifndef TIMECOLLECTOR_H
#define TIMECOLLECTOR_H

#include <vector>
#include <mutex>
#include <chrono>

class TimeCollectorEntry
{

public:
    TimeCollectorEntry(int state, bool autoCollect = true);
    ~TimeCollectorEntry();

    std::chrono::high_resolution_clock::duration Stop();

private:
    int _state;
    bool _autoCollect;
    std::chrono::high_resolution_clock::time_point _startTime;

};

class TimeCollector
{

public:
    static void Initialize(int states);
    static void AddToTimeCollector(int state, std::chrono::high_resolution_clock::duration time);
    static std::chrono::high_resolution_clock::duration GetTimeValue(int state);

private:
    static std::vector<std::chrono::high_resolution_clock::duration> _timeCollectors;
    static std::mutex _mutex;

};

#endif // TIMECOLLECTOR_H
