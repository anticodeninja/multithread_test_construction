#include "timecollector.h"

#include <thread>
#include <iostream>

#include "global_settings.h"

time_point TimeCollector::_initializeTime;
std::vector<TimeBaseEntry> TimeCollector::_timeCollectors;
std::mutex TimeCollector::_mutex;

TimeCollectorEntry::TimeCollectorEntry(int state)
    : _state(state),
      _startTime(std::chrono::high_resolution_clock::now()),
      _collected(false)
{
}

TimeCollectorEntry::~TimeCollectorEntry()
{
    Stop();
}

void TimeCollectorEntry::Stop()
{
    if(_collected)
        return;
    _collected = true;

    _endTime = std::chrono::high_resolution_clock::now();
    TimeCollector::AddToTimeCollector(*this);
}

void TimeCollector::Initialize(int reserve)
{
    _initializeTime = std::chrono::high_resolution_clock::now();
    _timeCollectors.reserve(reserve);
}

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    TAKE_LOCK(_mutex);
    _timeCollectors.push_back(TimeBaseEntry(entry.getState(), entry.getStartTime(), entry.getEndTime()));
}

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << _timeCollectors.size() << std::endl;
    for(auto i = _timeCollectors.begin(); i != _timeCollectors.end(); ++i) {
        stream << i->getState() << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getStartTime() - _initializeTime).count();
        stream << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getEndTime() - _initializeTime).count();
        stream << std::endl;
    }
}
