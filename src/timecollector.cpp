#include "timecollector.h"

#include <map>
#include <thread>
#include <iostream>

#include "global_settings.h"

time_point TimeCollector::_initializeTime;

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
const int PREDICTED_ELEMENTS = 1048576;
std::vector<TimeBaseEntry> TimeCollector::_timeEntries;
std::mutex TimeCollector::_mutex;
#else
std::atomic<ulong> TimeCollector::_totalTime[static_cast<int>(Counters::CountersCount)];
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
    _timeEntries.reserve(PREDICTED_ELEMENTS);
#endif
}

#if TIME_PROFILE >= 2
void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    _mutex.lock();
    _timeEntries.push_back(TimeBaseEntry(entry.getCounter(), entry.getThreadId(),
                                         entry.getStartTime(), entry.getEndTime()));
    _mutex.unlock();
}

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << "// Verbose: 2" << std::endl;
    stream << _timeEntries.size() << std::endl;

    auto humanThreadId = 1;
    std::map<std::thread::id, int> humanThreads;

    for(auto i = _timeEntries.begin(); i != _timeEntries.end(); ++i) {
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
    _totalTime[static_cast<int>(entry.getCounter())]
        .fetch_add(
                   std::chrono::duration_cast<time_duration>(entry.getEndTime() - entry.getStartTime()).count(),
                   std::memory_order_acq_rel);
}

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << "// Verbose: 1" << std::endl;
    stream << static_cast<int>(Counters::CountersCount) << std::endl;

    for(auto i = 0; i < static_cast<int>(Counters::CountersCount); ++i) {
        stream << counterNames[static_cast<Counters>(i)] << " ";
        stream << _totalTime[i];
        stream << std::endl;
    }
}
#endif
