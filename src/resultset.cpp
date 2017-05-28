#include "resultset.hpp"

#include <numeric>

ResultSet::ResultSet(set_size_t limit, feature_size_t featuresLen) :
    _limit(limit),
    _featuresLen(featuresLen),
    _size(0),
    _results(new test_feature_t[limit * featuresLen]),
    _resultsCosts(new feature_size_t[limit]),
    _resultsHashes(new calc_hash_t[limit]),
    _costBarrier(featuresLen + 1)
{
}

ResultSet::~ResultSet() {
    delete [] _results;
    delete [] _resultsCosts;
    delete [] _resultsHashes;
}

bool ResultSet::append(test_feature_t* covering) {
    feature_size_t cost = std::accumulate(&covering[0], &covering[_featuresLen], 0);
    if (cost > _costBarrier) {
        return false;
    }

    calc_hash_t hash = calcHash(covering);

    set_size_t index = 0;
    while (index < _size && _resultsCosts[index] <= cost) {
        if (isInclude(&_results[index * _featuresLen], _resultsHashes[index],
                      covering, hash)) {
            return false;
        }
        index += 1;
    }

    while (index < _size) {
        if (isInclude(covering, hash,
                      &_results[index * _featuresLen], _resultsHashes[index])) {
            for (set_size_t i = index; i < (_size - 1); ++i) {
                for (feature_size_t j = 0; j < _featuresLen; ++j) {
                    _results[i * _featuresLen + j] = _results[(i+1) * _featuresLen + j];
                }
                _resultsCosts[i] = _resultsCosts[i+1];
                _resultsHashes[i] = _resultsHashes[i+1];
            }

            _size -= 1;
            continue;
        }

        index += 1;
    }

    index = _size;
    for (set_size_t i = 0; i < _size; ++i) {
        if (cost < _resultsCosts[i]) {
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
        _resultsCosts[i] = _resultsCosts[i-1];
        _resultsHashes[i] = _resultsHashes[i-1];
    }

    for (feature_size_t j = 0; j < _featuresLen; ++j) {
        _results[index * _featuresLen + j] = covering[j];
    }
    _resultsCosts[index] = cost;
    _resultsHashes[index] = hash;

    if (_size < _limit) {
        _size += 1;
    } else {
        _costBarrier = _resultsCosts[_limit - 1];
    }

    return true;
}

calc_hash_t ResultSet::calcHash(test_feature_t* first) const {
    calc_hash_t temp = 0;
    for (feature_size_t i = 0; i < _featuresLen; ++i) {
        if (first[i]) {
            temp |= 1 << (i % calc_hash_bits);
        }
    }
    return temp;
}

bool ResultSet::isInclude(test_feature_t* first, calc_hash_t firstHash,
                          test_feature_t* second, calc_hash_t secondHash) const {

    if ((firstHash & secondHash) == firstHash) {
        return true;
    }

    for (feature_size_t i = 0; i < _featuresLen; ++i) {
        if (first[i] > second[i]) {
            return false;
        }
    }

    return true;
}
