#include "optimal_plan.h"

#include <iostream>
#include <limits>

#include "global_settings.h"

OptimalPlan::OptimalPlan(int* counts, int len)
    : _counts(counts),
      _indexes(len),
      _lastIndexes(len),
      _calcIndexes(len),
      _optimalPlan(len)
{
    for(auto i=0; i<len; ++i) {
        _indexes[i] = i;
    }
}

int OptimalPlan::FindNextStep(int begin, int end)
{
    auto max = end - begin;
    auto len = max / 2;
    auto median = begin + len;
    auto bestComplexity = std::numeric_limits<int>::max();

    auto p = 0;
    for(auto i = begin; i <= median; ++i, ++p) {
        _calcIndexes[i] = p;
    }

    while(p >= 0) {
        // Generate second part of combination
        auto c = 0, ci = begin;
        for(auto i=median + 1; i<=end; ++i) {
            for(;c == _calcIndexes[ci] && ci <= median; ++c, ++ci);
            _calcIndexes[i] = c++;
        }

        // Calculate complexity
        auto sum1 = 0, sum2 = 0, complexity = 0;
        for(auto i=begin; i<=median; ++i) {
            sum1 += _counts[_indexes[begin + _calcIndexes[i]]];
        }
        for(auto i=median + 1; i<=end; ++i) {
            sum2 += _counts[_indexes[begin + _calcIndexes[i]]];
        }
        complexity = sum1 > sum2 ? sum1 - sum2 : sum2 - sum1;

        // Save best step
        if(complexity < bestComplexity) {
            bestComplexity = complexity;
            for(auto i=begin; i<=end; ++i) {
                _optimalPlan[i] = _calcIndexes[i];
            }
        }

        // Generate next combination
        if(_calcIndexes[median] == max) {
            p -= 1;
        } else {
            p = len;
        }

        if(p >= 0) {
            for(auto i=len; i >= p; --i) {
                _calcIndexes[begin + i] = _calcIndexes[begin + p] + i - p + 1;
            }
        }
    }

    for(auto i=begin; i<=end; ++i) {
        _lastIndexes[i] = _indexes[i];
    }
    for(auto i=begin; i<=end; ++i) {
        _indexes[i] = _lastIndexes[begin + _optimalPlan[i]];
    }

    // Print
#ifdef DEBUG_MODE
    std::cout << "Optimal plan: " << std::endl;
    std::cout << begin << " " << median << " " << end << " | ";
    for(auto i=0; i<_indexes.size(); ++i)
        std::cout << _indexes[i] << " ";
    std::cout << "| ";
    for(auto i=0; i<_indexes.size(); ++i)
        std::cout << _counts[_indexes[i]] << " ";
    std::cout << "| " << bestComplexity << std::endl;
#endif

    return median;
}


