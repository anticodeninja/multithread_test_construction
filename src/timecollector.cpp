#include "timecollector.h"

#include <map>
#include <thread>
#include <iostream>

#include "global_settings.h"

time_point TimeCollector::_initializeTime;
std::mutex TimeCollector::_mutex;

std::map<Counters, std::string> counterNames = {
    { Counters::All, "All"},
    { Counters::ReadingInput, "ReadingInput"},
    { Counters::PreparingInput, "PreparingInput"},
    { Counters::PlanBuilding, "PlanBuilding"},
    { Counters::QHandling, "QHandling"},
    { Counters::RMerging, "RMerging"},
    { Counters::WritingOutput, "WritingOutput"},
    { Counters::Threading, "Threading"},
    { Counters::CrossThreading, "CrossThreading"}
};

#if TIME_PROFILE >= 2
const int GLOBAL_RESERVATION = 1048576;
const int THREAD_RESERVATION = 1024;
std::vector<TimeBaseEntry> TimeCollector::_globalList;
thread_local std::vector<TimeBaseEntry> TimeCollector::_threadList;
#else
std::vector<ulong> TimeCollector::_globalTotal;
thread_local std::vector<ulong> TimeCollector::_threadTotal;
#endif

TimeCollectorEntry::TimeCollectorEntry(Counters counter)
    : _counter(counter),
      _startTime(std::chrono::steady_clock::now()),
      _collected(false)
#if TIME_PROFILE >= 2
    , _threadId(std::this_thread::get_id())
#endif
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

void TimeCollector::Initialize()
{
    _initializeTime = std::chrono::steady_clock::now();
#if TIME_PROFILE >= 2
    _globalList.reserve(GLOBAL_RESERVATION);
#else
    _globalTotal.resize(static_cast<int>(Counters::CountersCount));
#endif
}

#if TIME_PROFILE >= 2

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    _threadList.push_back(TimeBaseEntry(entry.getCounter(), entry.getThreadId(),
                                         entry.getStartTime(), entry.getEndTime()));
}

void TimeCollector::ThreadInitialize()
{
    _threadList.reserve(THREAD_RESERVATION);
}

void TimeCollector::ThreadFinalize()
{
    _mutex.lock();

    std::move(_threadList.begin(), _threadList.end(), std::back_inserter(_globalList));
    _threadList.clear();

    _mutex.unlock();
}

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << "// Verbose: 2" << std::endl;
    stream << _globalList.size() << std::endl;

    auto humanThreadId = 1;
    std::map<std::thread::id, int> humanThreads;

    for(auto i = _globalList.begin(); i != _globalList.end(); ++i) {
        auto tempId = humanThreads.find(i->getThreadId());
        if(tempId == humanThreads.end())
        {
            tempId = humanThreads.insert({i->getThreadId(), humanThreadId++}).first;
        }

        stream << tempId->second << " ";
        stream << counterNames[i->getCounter()] << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getStartTime() - _initializeTime).count();
        stream << " ";
        stream << std::chrono::duration_cast<time_duration>(i->getEndTime() - _initializeTime).count();
        stream << std::endl;
    }
}

#else

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    _threadTotal[static_cast<int>(entry.getCounter())] += 
                   std::chrono::duration_cast<time_duration>(entry.getEndTime() - entry.getStartTime()).count();
}

void TimeCollector::ThreadInitialize()
{
    _threadTotal.resize(static_cast<int>(Counters::CountersCount));
}

void TimeCollector::ThreadFinalize()
{
    _mutex.lock();

    for(auto i = 0; i < static_cast<int>(Counters::CountersCount); ++i) {
        _globalTotal[i] += _threadTotal[i];
        _threadTotal[i] = 0;
    }

    _mutex.unlock();
}

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << "// Verbose: 1" << std::endl;
    stream << static_cast<int>(Counters::CountersCount) << std::endl;

    for(auto i = 0; i < static_cast<int>(Counters::CountersCount); ++i) {
        stream << counterNames[static_cast<Counters>(i)] << " ";
        stream << _globalTotal[i];
        stream << std::endl;
    }
}

#endif
