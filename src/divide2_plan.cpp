#include "divide2_plan.h"

#include <iostream>
#include <limits>

#include "global_settings.h"

Divide2Plan::Divide2Plan(int* counts, int len)
    : _counts(counts),
      _indexes(len),
      _lastIndexes(len)
{
    for(auto i=0; i<len; ++i) {
        _indexes[i] = i;
    }
}

int Divide2Plan::FindNextStep(int begin, int end)
{
    auto max = end - begin;
    auto len = max / 2;
    auto median = begin + len;

    auto p = 0;
    for(auto i = begin; i <= end; ++i) {
        _lastIndexes[i] = _indexes[i];
    }

    auto sum1 = 0;
    auto index1 = begin;
    auto sum2 = 0;
    auto index2 = median + 1;

    for(auto i = begin; i <= end; ++i) {
        if(sum1 <= sum2 || index2 == end + 1) {
            sum1 += _counts[_lastIndexes[i]];
            _indexes[index1] = _lastIndexes[i];
            index1 += 1;
        } else {
            sum2 += _counts[_lastIndexes[i]];
            _indexes[index2] = _lastIndexes[i];
            index2 += 1;
        }
    }

    DEBUG_BLOCK (
       getDebugStream() << "Divide2Plan: " << std::endl;
       
       getDebugStream() << begin << " " << median << " " << end << " | ";
       for(auto i=0; i<_indexes.size(); ++i)
           getDebugStream() << _indexes[i] << " ";
       
       getDebugStream() << "| ";
       for(auto i=0; i<_indexes.size(); ++i)
           getDebugStream() << _counts[_indexes[i]] << " ";
       
       getDebugStream() << "| " << (sum1 > sum2 ? sum1 - sum2 : sum2 - sum1) << std::endl;
    )

    return median;
}
