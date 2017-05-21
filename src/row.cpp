#include "row.hpp"

#include <stdexcept>
#include <cmath>
#include <limits>

#include "global_settings.h"

const int SKIP_VALUE = std::numeric_limits<int>::min();

Row::Row()
    : _width(0),
      _values(nullptr)
{
}

Row::Row(int width)
    : _width(width),
      _values(new int[width])
{
}

Row::Row(Row &&row) {
    _values = row._values;
    _width = row._width;

    row._values = nullptr;
    row._width = 0;
}

Row& Row::operator=(Row &&row) {
    if(_values != nullptr)
        delete [] _values;

    _values = row._values;
    _width = row._width;

    row._values = nullptr;
    row._width = 0;
}

Row::~Row()
{
    if(_values != nullptr)
        delete[] _values;
    _values = nullptr;
}

Row Row::createAsDifference(const WorkRow &w1, const WorkRow &w2)
{
    if(w1.getWidth() != w2.getWidth())
        throw std::invalid_argument("Widths aren't equal");

    Row temp(w1.getWidth());
    for(auto i=0; i<w1.getWidth(); ++i) {
        if(w1.getValue(i) == SKIP_VALUE || w2.getValue(i) == SKIP_VALUE)
            temp.setValue(i, 0);
        else
            temp.setValue(i, std::abs(w1.getValue(i) - w2.getValue(i)));
    }
    return temp;
}

bool Row::isInclude(const Row &row) const
{
    if(getWidth() != row.getWidth())
        throw std::invalid_argument("Widths aren't equal");

    for(auto i=0; i<getWidth(); ++i) {
        if(row.getValue(i) < getValue(i)) {
            return false;
        }
    }

    return true;
}

std::ostream& operator<<(std::ostream& stream, const Row& row)
{
    for(auto i=0; i<row.getWidth(); ++i) {
        stream << row.getValue(i) << " ";
    }
    return stream;
}
