#include "timecollector.h"

#include <map>

#include <thread>
#include <iostream>

#include "global_settings.h"

time_point TimeCollector::_initializeTime;
std::vector<TimeBaseEntry> TimeCollector::_timeCollectors;
std::mutex TimeCollector::_mutex;

TimeCollectorEntry::TimeCollectorEntry(int state)
    : _state(state),
      _threadId(std::this_thread::get_id()),
      _startTime(std::chrono::steady_clock::now()),
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

    _endTime = std::chrono::steady_clock::now();
    TimeCollector::AddToTimeCollector(*this);
}

void TimeCollector::Initialize(int reserve)
{
    _initializeTime = std::chrono::steady_clock::now();
    _timeCollectors.reserve(reserve);
}

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _timeCollectors.push_back(TimeBaseEntry(entry.getState(), entry.getThreadId(),
                                            entry.getStartTime(), entry.getEndTime()));
}

void TimeCollector::PrintInfo(std::ostream &stream, std::map<int, std::string> &states)
{
    stream << _timeCollectors.size() << std::endl;

    auto humanThreadId = 1;
    std::map<std::thread::id, int> humanThreads;

    for(auto i = _timeCollectors.begin(); i != _timeCollectors.end(); ++i) {
        auto tempId = humanThreads.find(i->getThreadId());
        if(tempId == humanThreads.end())
        {
            tempId = humanThreads.insert({i->getThreadId(), humanThreadId++}).first;
        }

        stream << tempId->second << " ";
        stream << states[i->getState()] << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getStartTime() - _initializeTime).count();
        stream << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getEndTime() - _initializeTime).count();
        stream << std::endl;
    }
}
