#ifndef IRREDUNDANT_MATRIX_BASE_H
#define IRREDUNDANT_MATRIX_BASE_H

#include <iostream>
#include <functional>
#include <mutex>

#include "row.h"

class IrredundantMatrixBase
{
public:
    virtual void addRow(Row&& row, bool concurrent) = 0;
    virtual void enumerate(std::function<void (Row&)> callback) = 0;
    virtual void addMatrix(IrredundantMatrixBase&& matrix, bool concurrent);
    virtual void printMatrix(std::ostream& stream);

    virtual int getHeight() = 0;
    virtual int getWidth() = 0;

protected:
    std::mutex _mutex;

};

#endif // IRREDUNDANT_MATRIX_BASE_H
