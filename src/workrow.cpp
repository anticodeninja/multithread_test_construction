#include "workrow.hpp"

#include <stdexcept>

WorkRow::WorkRow(int* matrix, int index, int width) {
    _matrix = matrix;
    _offset = index * width;
    _width = width;
}

bool WorkRow::operator<(const WorkRow &rhs) const {
    if(_width != rhs._width)
        throw std::invalid_argument("Widthes mismatched");

    for(auto i=0; i<_width; ++i) {
        if(getValue(i) == rhs.getValue(i)) {
            continue;
        }
        return getValue(i) < rhs.getValue(i);
    }

    return false;
}
