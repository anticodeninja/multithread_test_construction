#ifndef DIVIDE2_PLAN_H
#define DIVIDE2_PLAN_H

#include <vector>

class Divide2Plan
{
public:
    Divide2Plan(int* counts, int len);

    int FindNextStep(int begin, int end);

    int GetIndex(int index) {
        return _indexes[index];
    }

private:
    int* _counts;
    std::vector<int> _indexes;
    std::vector<int> _lastIndexes;
};

#endif // Divide2_PLAN_H
