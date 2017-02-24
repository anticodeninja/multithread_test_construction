#ifndef RESULTSET_H
#define RESULTSET_H

#include <iostream>
#include <vector>

class Result {
    
 public:
    
    Result(uint_fast8_t* mask, uint_fast16_t length) {
        _mask = new uint_fast8_t[length];
        _length = length;
        _cost = 0;
        
        for (uint_fast16_t i = 0; i < length; ++i) {
            _mask[i] = mask[i];
            _cost += mask[i];
        }
    }

    Result(const Result& result) = delete;
    Result& operator=(const Result&) = delete;

    Result(Result&& other) {
        _mask = other._mask;
        _length = other._length;
        _cost = other._cost;

        other._mask = nullptr;
    }

    virtual ~Result() {
        if (_mask != nullptr) {
            delete [] _mask;
        }
    }

    inline uint_fast16_t getCost() const {
        return _cost;
    }

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

    friend std::ostream& operator<<(std::ostream& os, const Result& result);

 private:

    uint_fast8_t* _mask;
    uint_fast16_t _length;
    uint_fast16_t _cost;
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

    friend std::ostream& operator<<(std::ostream& os, const ResultSet& resultSet);

 private:

    Result** _results;
    uint_fast16_t _size;
    uint_fast16_t _limit;
};

std::ostream& operator<<(std::ostream& os, const Result& result) {
    for (uint_fast16_t i = 0; i < result._length; ++i) {
        os << (int)result._mask[i] << " ";
    }
    os << "| " << result._cost;
    
    return os;
}

std::ostream& operator<<(std::ostream& os, const ResultSet& resultSet) {
    for(uint_fast16_t i=0; i<resultSet._size; ++i) {
        std::cout << *resultSet._results[i] << std::endl;
    }
    
    return os;
}

#endif // RESULTSET_H
