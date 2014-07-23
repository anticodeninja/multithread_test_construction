#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <deque>

#include "row.h"

class IrredundantMatrix
{

public:
    IrredundantMatrix();
    void addRow(const Row& row);
    void printMatrix(std::ostream& stream, bool printSize = false);

private:
    std::deque<Row> _rows;

};

#endif // IRREDUNDANTMATRIX_H
