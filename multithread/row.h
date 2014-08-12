#ifndef ROW_H
#define ROW_H

#include <utility>

#include "workrow.h"

class Row
{
public:
    Row();
    Row(int width);

    Row(Row&& row);
    Row& operator=(Row&& row);

    Row(const Row& row) = delete;
    Row& operator=(const Row& right) = delete;

    ~Row();

public:
    static Row createAsDifference(const WorkRow& w1, const WorkRow& w2);
    bool isInclude(const Row& row) const;

    inline int getValue(int index) const {
        return _values[index];
    }

    inline void setValue(int index, int value) {
        _values[index] = value;
    }

    inline unsigned int getWidth() const {
        return _width;
    }

private:
    int* _values;
    int _width;
};

#endif // ROW_H
