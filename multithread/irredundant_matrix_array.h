#ifndef IRREDUNDANT_MATRIX_ARRAY_H
#define IRREDUNDANT_MATRIX_ARRAY_H

#include <iostream>
#include <vector>
#include <mutex>

#include "irredundant_matrix_base.h"

class IrredundantMatrixArray : public IrredundantMatrixBase
{
public:
    IrredundantMatrixArray();
    void addRow(Row row, bool concurrent);
    void printMatrix(std::ostream& stream);

private:
    std::vector<Row> _rows;
    std::mutex _mutex;

};

#endif // IRREDUNDANT_MATRIX_ARRAY_H
