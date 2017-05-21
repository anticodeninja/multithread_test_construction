#ifndef MANY_WORKERS_PLAN_H
#define MANY_WORKERS_PLAN_H

#include <vector>
#include <mutex>

class ManyWorkersTask
{
 public:

    ManyWorkersTask(int first, int second, int weight, bool isEmpty)
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

class ManyWorkersPlan
{
 public:
    ManyWorkersPlan(int* counts, int len);
    ~ManyWorkersPlan();

    ManyWorkersTask* getTask();

 private:
    std::mutex _mutex;
    std::vector<ManyWorkersTask> _tasks;
    ManyWorkersTask* _emptyTask;
    int _current;
};

#endif // MANY_WORKERS_PLAN_H
