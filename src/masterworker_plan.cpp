#include "masterworker_plan.h"

#include <algorithm>

#include "global_settings.h"
#include "timecollector.h"

MasterWorkerPlan::MasterWorkerPlan(int* counts, int len)
    : _emptyTask(new MasterWorkerTask(0, 0, 0, true)),
    _current(0)
{
    START_COLLECT_TIME(planBuilding, Counters::PlanBuilding);

    for(auto i=0; i<len-1; ++i) {
        for(auto j=i+1; j<len; ++j) {
            _tasks.push_back(MasterWorkerTask(i, j, counts[i] * counts[j], false));
        }
    }

    std::sort(_tasks.begin(), _tasks.end(), 
              [](const MasterWorkerTask &a, const MasterWorkerTask &b) -> bool
              { 
                  return a.getWeight() > b.getWeight();
              });

    DEBUG_BLOCK (
       getDebugStream() << "MasterWorkerPlan: " << std::endl;
       for(auto i=0; i<_tasks.size(); ++i) {
           auto &task = _tasks[i];
           getDebugStream() << task.getFirst() << "-" << task.getSecond() << ":" << task.getWeight() << std::endl;
       }
    )

    STOP_COLLECT_TIME(planBuilding);
}

MasterWorkerPlan::~MasterWorkerPlan()
{
    delete _emptyTask;
}

MasterWorkerTask* MasterWorkerPlan::getTask()
{
    START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
    std::unique_lock<std::mutex> lock(_mutex);
    STOP_COLLECT_TIME(crossThreading);

    if (_current != _tasks.size()) {
        return &_tasks[_current++];
    } else {
        return _emptyTask;
    }
}
