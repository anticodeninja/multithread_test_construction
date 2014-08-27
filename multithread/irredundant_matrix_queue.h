#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <deque>
#include <mutex>

#include "row.h"
#include "irredundant_matrix_base.h"

class IrredundantMatrixQueue : public IrredundantMatrixBase
{

public:
    IrredundantMatrixQueue();
    void addRow(Row &&row, bool concurrent);

    IrredundantMatrixQueue(IrredundantMatrixQueue& matrix) = delete;
    IrredundantMatrixQueue& operator=(IrredundantMatrixQueue& matrix) = delete;

private:
    std::deque<Row> _rows;
    std::mutex _mutex;

};

#endif // IRREDUNDANTMATRIX_H
