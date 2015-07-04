#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <mutex>

#ifdef IRREDUNTANT_VECTOR
#include <vector>
#else
#include <deque>
#endif

#include "row.h"

class IrredundantMatrix
{

public:
    IrredundantMatrix();
    void addRow(Row &&row, bool concurrent);
    void addMatrix(IrredundantMatrix&& matrix, bool concurrent);
    void printMatrix(std::ostream& stream);

    int getHeight();
    int getWidth();

    IrredundantMatrix(IrredundantMatrix& matrix) = delete;
    IrredundantMatrix& operator=(IrredundantMatrix& matrix) = delete;

private:

#ifdef IRREDUNTANT_VECTOR
    std::vector<Row> _rows;
#else
    std::deque<Row> _rows;
#endif

    std::mutex _mutex;
};

#endif // IRREDUNDANTMATRIX_H
