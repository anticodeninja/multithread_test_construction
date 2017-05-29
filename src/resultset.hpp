#pragma once

#include "global_settings.h"

class ResultSet final {

 public:
    ResultSet(set_size_t limit, feature_size_t featureLen);
    ~ResultSet();

    ResultSet(const ResultSet& result) = delete;
    ResultSet& operator=(const ResultSet&) = delete;

    bool append(test_feature_t* covering);

    inline set_size_t getSize() const { return _size; }
    inline feature_size_t getCostBarrier() const { return _costBarrier; }
    inline test_feature_t* get(set_size_t index) const { return &_results[index * _featuresLen]; }
    inline bool isFull() const { return _size == _limit; }

 private:
    bool isInclude(test_feature_t* first, calc_hash_t firstHash,
                   test_feature_t* second, calc_hash_t secondHash) const;
    calc_hash_t calcHash(test_feature_t* first) const;

    test_feature_t* _results;
    feature_size_t* _resultsCosts;
    calc_hash_t* _resultsHashes;

    set_size_t _size;
    feature_size_t _featuresLen;
    set_size_t _limit;
    feature_size_t _costBarrier;
};
