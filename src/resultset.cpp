#include "resultset.hpp"

Result::Result(uint_fast8_t* mask, uint_fast16_t length) {
    _mask = new uint_fast8_t[length];
    _length = length;
    _cost = 0;

    for (uint_fast16_t i = 0; i < length; ++i) {
        _mask[i] = mask[i];
        _cost += mask[i];
    }
}

Result::~Result() {
    if (_mask != nullptr) {
        delete [] _mask;
    }
}

Result::Result(Result&& other) {
    _mask = other._mask;
    _length = other._length;
    _cost = other._cost;

    other._mask = nullptr;
}
