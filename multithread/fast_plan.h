#ifndef FAST_PLAN_H
#define FAST_PLAN_H

#include <vector>

class FastPlan
{
public:
    FastPlan(int* counts, int len);


    int FindNextStep(int begin, int end);


    int GetIndex(int index) {
        return _indexes[index];
    }

private:
    int* _counts;
    std::vector<int> _indexes;
    std::vector<int> _lastIndexes;
};

#endif // FAST_PLAN_H
