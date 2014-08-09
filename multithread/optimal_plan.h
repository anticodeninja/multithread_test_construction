#ifndef OPTIMAL_PLAN_H
#define OPTIMAL_PLAN_H

#include <cstddef>
#include <vector>

class OptimalPlan
{
public:
    OptimalPlan(int* counts, int len);


    int FindNextStep(int begin, int end);


    int GetIndex(int index) {
        return _indexes[index];
    }

private:
    int* _counts;
    std::vector<int> _indexes;
    std::vector<int> _lastIndexes;
    std::vector<int> _calcIndexes;
    std::vector<int> _optimalPlan;
};

#endif // OPTIMAL_PLAN_H
