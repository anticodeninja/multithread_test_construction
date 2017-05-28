#include "resultset.hpp"

#include <limits>
#include <numeric>

const feature_size_t MAX_COST = std::numeric_limits<feature_size_t>::max();

ResultSet::ResultSet(set_size_t limit, feature_size_t featuresLen) :
    _limit(limit),
    _featuresLen(featuresLen),
    _size(0),
    _results(new test_feature_t[limit * featuresLen]),
    _resultsCost(new feature_size_t[limit]),
    _costBarrier(MAX_COST)
{
}

ResultSet::~ResultSet() {
    delete [] _results;
    delete [] _resultsCost;
}

bool ResultSet::append(feature_size_t cost, test_feature_t* covering) {
    if (cost > _costBarrier) {
        return false;
    }

    set_size_t index = 0;
    while (index < _size && _resultsCost[index] <= cost) {
        if (isInclude(&_results[index * _featuresLen], covering)) {
            return false;
        }
        index += 1;
    }

    while (index < _size) {
        if (isInclude(covering, &_results[index * _featuresLen])) {
            for (set_size_t i = index; i < (_size - 1); ++i) {
                for (feature_size_t j = 0; j < _featuresLen; ++j) {
                    _results[i * _featuresLen + j] = _results[(i+1) * _featuresLen + j];
                }
                _resultsCost[i] = _resultsCost[i+1];
            }

            _size -= 1;
            continue;
        }

        index += 1;
    }

    index = _size;
    for (set_size_t i = 0; i < _size; ++i) {
        if (cost < _resultsCost[i]) {
            index = i;
            break;
        }
    }

    if (index >= _limit) {
        return false;
    }

    for (set_size_t i = std::min(_size, _limit-1); i > index; --i) {
        for (feature_size_t j = 0; j < _featuresLen; ++j) {
            _results[i * _featuresLen + j] = _results[(i-1) * _featuresLen + j];
        }
        _resultsCost[i] = _resultsCost[i-1];
    }

    for (feature_size_t j = 0; j < _featuresLen; ++j) {
        _results[index * _featuresLen + j] = covering[j];
    }
    _resultsCost[index] = cost;

    if (_size < _limit) {
        _size += 1;
    } else {
        _costBarrier = _resultsCost[_limit - 1];
    }

    return true;
}

bool ResultSet::isInclude(test_feature_t* first, test_feature_t* second) const {
    for (feature_size_t i = 0; i < _featuresLen; ++i) {
        if (first[i] > second[i]) {
            return false;
        }
    }

    return true;
}
