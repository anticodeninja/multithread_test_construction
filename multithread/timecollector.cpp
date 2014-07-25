#include "timecollector.h"

#include <thread>
#include <iostream>

TimeCollectorEntry::TimeCollectorEntry(int state, bool autoCollect)
{
    _autoCollect = autoCollect;
    _state = state;
    _startTime = std::chrono::high_resolution_clock::now();
}

TimeCollectorEntry::~TimeCollectorEntry()
{
    Stop();
}

std::chrono::high_resolution_clock::duration TimeCollectorEntry::Stop()
{
    std::chrono::high_resolution_clock::duration duration =
            std::chrono::high_resolution_clock::duration::zero();

    if(_startTime != std::chrono::high_resolution_clock::time_point::min())
    {
        duration = std::chrono::high_resolution_clock::now() - _startTime;
        _startTime = std::chrono::high_resolution_clock::time_point::min();
    }

    if(_autoCollect)
        TimeCollector::AddToTimeCollector(_state, duration);
    return duration;
}

std::vector<std::chrono::high_resolution_clock::duration> TimeCollector::_timeCollectors;
std::mutex TimeCollector::_mutex;

void TimeCollector::Initialize(int states)
{
    _timeCollectors.resize(states);
}

void TimeCollector::AddToTimeCollector(int state, std::chrono::high_resolution_clock::duration time)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _timeCollectors[state] += time;
}

std::chrono::high_resolution_clock::duration TimeCollector::GetTimeValue(int state)
{
    return _timeCollectors[state];
}
