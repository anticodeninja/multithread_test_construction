#ifndef DIVIDE2_PLAN_H
#define DIVIDE2_PLAN_H

#include <vector>

class Divide2Task
{
 public:
    
    Divide2Task() :
        _firstWeight(0),
        _secondWeight(0) {}

    int getFirstSize() const {
        return _first.size();
    }

    int getSecondSize() const {
        return _second.size();
    }
    
    int getFirst(int id) const {
        return _first[id];
    }

    int getSecond(int id) const {
        return _second[id];
    }

    void append(int item, int weight) {
        if (_secondWeight > _firstWeight) {
            _first.push_back(item);
            _firstWeight += weight;
        } else {
            _second.push_back(item);
            _secondWeight += weight;
        }
    }

    int isEmpty() const {
        return getFirstSize() == 0 || getSecondSize() == 0;
    }
    
 private:
    
    std::vector<int> _first;
    std::vector<int> _second;
    int _firstWeight;
    int _secondWeight;
    
};

class Divide2Plan
{
public:
    Divide2Plan(int* counts, int len);

    Divide2Task* getTask(int step, int threadId) {
        return &_tasks[step][threadId];
    }

    int getStepsCount() const {
        return _steps;
    }

    int getMaxThreadsCount() const {
        return 1 << _steps;
    }

    int getThreadsCountForStep(int step) const {
        return 1 << step;
    }

private:
    int _steps;
    std::vector<std::vector<Divide2Task>> _tasks;
};

#endif // Divide2_PLAN_H
