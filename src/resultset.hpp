#ifndef RESULTSET_H
#define RESULTSET_H

#include <vector>

#include "datafile.hpp"

class Result {

public:

    Result(test_feature_t* mask, uint_fast16_t length);
    virtual ~Result();

    Result(Result&& other);

    Result(const Result& result) = delete;
    Result& operator=(const Result&) = delete;

    inline uint_fast16_t getCost() const { return _cost; }

    inline bool isInclude(const Result& other) const {
        if (_cost > other._cost) {
            return false;
        }

        for (uint_fast16_t i = 0; i < _length; ++i) {
            if (_mask[i] > other._mask[i]) {
                return false;
            }
        }

        return true;
    }

    inline test_feature_t get(feature_size_t id) const { return _mask[id]; }

 private:

    test_feature_t* _mask;
    feature_size_t _length;
    set_size_t _cost;
};

class ResultSet
{
 public:

    const static uint_fast16_t IGNORED = UINT_FAST16_MAX;

    ResultSet(uint_fast16_t limit) {
        _limit = limit;
        _size = 0;
        _results = new Result*[_limit];

        for (uint_fast16_t i = 0; i < _limit; ++i) {
            _results[i] = nullptr;
        }
    }

    ResultSet(const ResultSet& result) = delete;
    ResultSet& operator=(const ResultSet&) = delete;

    virtual ~ResultSet() {
        for (uint_fast16_t i = 0; i < _size; ++i) {
            delete _results[i];
        }
        delete [] _results;
    }

    inline uint_fast16_t getSize() const {
        return _size;
    }

    inline const Result& get(uint_fast16_t index) const {
        return *_results[index];
    }

    inline bool isFull() const {
        return _size == _limit;
    }

    inline uint_fast16_t append(Result&& result) {
        uint_fast16_t index = 0;
        while (index < _size) {
            if (_results[index]->isInclude(result)) {
                return IGNORED;
            }

            if (result.isInclude(*_results[index])) {
                delete _results[index];

                for (uint_fast16_t i = index; i < (_size - 1); ++i) {
                    _results[i] = _results[i+1];
                }

                _results[_size - 1] = nullptr;
                _size -= 1;

                continue;
            }

            index += 1;
        }

        index = _size;
        for (uint_fast16_t i = 0; i < _size; ++i) {
            if (result.getCost() < _results[i]->getCost()) {
                index = i;
                break;
            }
        }

        if (index < _limit) {
            if (_results[_limit - 1] != nullptr) {
                delete _results[_limit - 1];
            }

            for (uint_fast16_t i = _limit - 1; i > index; --i) {
                _results[i] = _results[i-1];
            }

            _results[index] = new Result(std::move(result));
            if (_size < _limit) {
                _size += 1;
            }

            return index;
        } else {
            return IGNORED;
        }
    }

 private:

    Result** _results;
    uint_fast16_t _size;
    uint_fast16_t _limit;
};

#endif // RESULTSET_H
