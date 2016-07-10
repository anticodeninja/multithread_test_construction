#include "timecollector.h"

#include <map>
#include <thread>
#include <chrono>
#include <iostream>

#include "global_settings.h"

const int GLOBAL_RESERVATION = 1048576;
const int THREAD_RESERVATION = 1024;

typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::nanoseconds time_duration;

std::vector<TimeBaseEntry> _globalList;
time_point _initializeTime;
std::mutex _mutex;
ulong _threadIdCounter;
std::map<std::thread::id, ulong> _threadIdCounterConv;

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
thread_local ulong _threadId;
thread_local std::vector<TimeBaseEntry> _threadList;
#else
thread_local ulong _threadStart;
thread_local std::vector<ulong> _threadTotal;
#endif

TimeCollectorEntry::TimeCollectorEntry(Counters counter)
    : _counter(counter),
      _start(TimeCollector::GetTickCount()),
      _length(0),
      _collected(false)
{
}

TimeCollectorEntry::~TimeCollectorEntry()
{
    Stop();
}

void TimeCollectorEntry::Pause()
{
    _length += TimeCollector::GetTickCount() - _start;
}

void TimeCollectorEntry::Continue()
{
    _start = TimeCollector::GetTickCount();
}

void TimeCollectorEntry::Stop()
{
    if(_collected)
        return;
    _collected = true;

    auto current = TimeCollector::GetTickCount();
    _length += current - _start;
    _start = current - _length; // Recalc "correct" start time

    if (_length == 0)
        return;
    
    TimeCollector::AddToTimeCollector(*this);
}

void TimeCollector::Initialize()
{
    _initializeTime = std::chrono::steady_clock::now();
    _globalList.clear();
    _globalList.reserve(GLOBAL_RESERVATION);
    _threadIdCounter = 1;
    _threadIdCounterConv.clear();
}

ulong TimeCollector::GetThreadId()
{
    auto threadId = std::this_thread::get_id();
    
    auto result = _threadIdCounterConv.find(threadId);
    if (result == _threadIdCounterConv.end())
        result = _threadIdCounterConv.insert({threadId, _threadIdCounter++}).first;

    return result->second;
}

ulong TimeCollector::GetTickCount()
{
    return std::chrono::duration_cast<time_duration>(std::chrono::steady_clock::now() - _initializeTime).count();
}

#if TIME_PROFILE >= 2

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    _threadList.push_back(TimeBaseEntry(entry.getCounter(), _threadId,
                                         entry.getStart(), entry.getLength()));
}

void TimeCollector::ThreadInitialize()
{
    _threadId = TimeCollector::GetThreadId();
    _threadList.reserve(THREAD_RESERVATION);
}

void TimeCollector::ThreadFinalize()
{
    _mutex.lock();

    std::move(_threadList.begin(), _threadList.end(), std::back_inserter(_globalList));
    _threadList.clear();

    _mutex.unlock();
}

#else

void TimeCollector::AddToTimeCollector(const TimeCollectorEntry &entry)
{
    _threadTotal[static_cast<int>(entry.getCounter())] += entry.getLength();
}

void TimeCollector::ThreadInitialize()
{
    _threadStart = TimeCollector::GetTickCount();
    _threadTotal.resize(static_cast<int>(Counters::CountersCount));
}

void TimeCollector::ThreadFinalize()
{
    _mutex.lock();

    for(auto i = 0; i < static_cast<int>(Counters::CountersCount); ++i) {
        _globalList.push_back(TimeBaseEntry(static_cast<Counters>(i), TimeCollector::GetThreadId(),
                                            _threadStart, _threadTotal[i]));
        _threadTotal[i] = 0;
    }

    _mutex.unlock();
}

#endif

void TimeCollector::PrintInfo(std::ostream &stream)
{
    stream << "// Verbose: 2" << std::endl;
    stream << _globalList.size() << std::endl;

    for(auto i = _globalList.begin(); i != _globalList.end(); ++i) {
        stream << i->getThreadId() << " ";
        stream << counterNames[i->getCounter()] << " ";
        stream << i->getStart();
        stream << " ";
        stream << (i->getStart() + i->getLength());
        stream << std::endl;
    }
}
