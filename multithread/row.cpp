#include "row.h"

#include <stdexcept>
#include <cmath>
#include <limits>
#include "workrow.h"

const int SKIP_VALUE = std::numeric_limits<int>::min();

Row::Row(const WorkRow& workRow) {
    _width = workRow.getWidth();
    _values = new int[_width];

    for(auto i=0; i<_width; ++i) {
        setValue(i, workRow.getValue(i));
    }
}

Row::Row(const Row &row)
{
    _width = row.getWidth();
    _values = new int[_width];

    for(auto i=0; i<_width; ++i) {
        setValue(i, row.getValue(i));
    }
}

Row::Row(int width)
{
    _width = width;
    _values = new int[_width];

    for(auto i=0; i<_width; ++i) {
        setValue(i, 0);
    }
}

Row& Row::operator=(const Row &right)
{
    if (this == &right) {
        return *this;
    }

    if(getWidth() != right.getWidth()) {
        delete[] _values;
        _width = right.getWidth();
        _values = new int[_width];
    }

    for(auto i=0; i<_width; ++i) {
        setValue(i, right.getValue(i));
    }
}

Row::~Row()
{
    delete[] _values;
}

Row Row::createAsDifference(const WorkRow &w1, const WorkRow &w2)
{
    if(w1.getWidth() != w2.getWidth())
        throw std::invalid_argument("Widths aren't equal");

    auto temp = Row(w1.getWidth());
    for(auto i=0; i<w1.getWidth(); ++i) {
        if(w1.getValue(i) == std::numeric_limits<int>::min() ||
           w2.getValue(i) == std::numeric_limits<int>::min())
            temp.setValue(i, std::numeric_limits<int>::min());
        else
            temp.setValue(i, abs(w1.getValue(i) - w2.getValue(i)));
    }
    return temp;
}

bool Row::isInclude(const Row &row) const
{
    if(getWidth() != row.getWidth())
        throw std::invalid_argument("Widths aren't equal");

    for(auto i=0; i<getWidth(); ++i) {
        if(
           (row.getValue(i) != SKIP_VALUE && getValue(i) != SKIP_VALUE && row.getValue(i) > getValue(i)) ||
           (row.getValue(i) != SKIP_VALUE && getValue(i) == SKIP_VALUE)
        ) {
            return false;
        }
    }

    return true;
}
