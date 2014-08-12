#ifndef IRREDUNDANT_MATRIX_BASE_H
#define IRREDUNDANT_MATRIX_BASE_H

#include <iostream>

#include "row.h"

class IrredundantMatrixBase
{
public:
    virtual void addRow(Row row, bool concurrent) = 0;
    virtual void printMatrix(std::ostream& stream) = 0;
};

#endif // IRREDUNDANT_MATRIX_BASE_H
