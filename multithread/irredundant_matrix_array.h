#ifndef IRREDUNDANT_MATRIX_ARRAY_H
#define IRREDUNDANT_MATRIX_ARRAY_H

#include <iostream>
#include <vector>

#include "irredundant_matrix_base.h"

class IrredundantMatrixArray : public IrredundantMatrixBase
{
public:
    IrredundantMatrixArray();
    virtual void addRow(Row&& row, bool concurrent);
    virtual void enumerate(std::function<void (Row&)> callback);

    virtual int getHeight();
    virtual int getWidth();

    IrredundantMatrixArray(IrredundantMatrixArray& matrix) = delete;
    IrredundantMatrixArray& operator=(IrredundantMatrixArray& matrix) = delete;

private:
    std::vector<Row> _rows;

};

#endif // IRREDUNDANT_MATRIX_ARRAY_H
