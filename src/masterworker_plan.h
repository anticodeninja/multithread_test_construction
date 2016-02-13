#ifndef MASTER_WORKER_PLAN_H
#define MASTER_WORKER_PLAN_H

#include <vector>
#include <mutex>

class MasterWorkerTask
{
 public:
    
    MasterWorkerTask(int first, int second, int weight, bool isEmpty)
        : _first(first), _second(second), _weight(weight), _isEmpty(isEmpty) { }

    int getFirst() const {
        return _first;
    }

    int getSecond() const {
        return _second;
    }

    int getWeight() const {
        return _weight;
    }

    int isEmpty() const {
        return _isEmpty;
    }
    
 private:
    
    int _first;
    int _second;
    int _weight;
    bool _isEmpty;
    
};

class MasterWorkerPlan
{
 public:
    MasterWorkerPlan(int* counts, int len);
    ~MasterWorkerPlan();

    MasterWorkerTask* getTask();

 private:
    std::mutex _mutex;
    std::vector<MasterWorkerTask> _tasks;
    MasterWorkerTask* _emptyTask;
    int _current;
};

#endif // MASTER_WORKER_PLAN_H
