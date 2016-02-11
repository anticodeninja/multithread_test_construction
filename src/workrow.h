#ifndef WORKROW_H
#define WORKROW_H

class WorkRow
{
public:
    WorkRow(int* matrix, int index, int width);

    bool operator<(const WorkRow& rhs) const;

public:
    inline int getValue(int index) const {
        return _matrix[_offset + index];
    }

    inline void setValue(int index, int value) {
        _matrix[_offset + index] = value;
    }

    inline unsigned int getWidth() const {
        return _width;
    }

private:
    int* _matrix;
    int _offset;
    int _width;
};

#endif // WORKROW_H
