#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <mutex>
#include <vector>

#ifndef IRREDUNTANT_VECTOR
#include <deque>
#endif

#include "row.h"

class IrredundantMatrix
{

public:
    IrredundantMatrix(int width);
    void addRow(Row&& row, int* r, bool concurrent);
    void addMatrix(IrredundantMatrix&& matrix, bool concurrent);
    void printMatrix(std::ostream& stream);
    void printR(std::ostream& stream);

    int getHeight();
    int getWidth();

    IrredundantMatrix(IrredundantMatrix& matrix) = delete;
    IrredundantMatrix& operator=(IrredundantMatrix& matrix) = delete;

private:

    void addRowInternal(Row &&row);
    
#ifdef IRREDUNTANT_VECTOR
    std::vector<Row> _rows;
#else
    std::deque<Row> _rows;
#endif

    int _width;
    std::vector<int> _r;
    std::mutex _mutex;
};

#endif // IRREDUNDANTMATRIX_H
