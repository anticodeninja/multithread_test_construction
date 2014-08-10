#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <deque>
#include <mutex>

#include "row.h"

class IrredundantMatrix
{

public:
    IrredundantMatrix();
    void addRow(const Row& row, bool concurrent);
    void printMatrix(std::ostream& stream, bool printSize = false);

private:
    std::deque<Row> _rows;
    std::mutex _mutex;

};

#endif // IRREDUNDANTMATRIX_H
