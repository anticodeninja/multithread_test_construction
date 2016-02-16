#ifndef IRREDUNDANTMATRIX_H
#define IRREDUNDANTMATRIX_H

#include <iostream>
#include <mutex>
#include <vector>

#ifndef IRREDUNTANT_VECTOR
#include <deque>
#endif

#include "row.h"

#ifdef USE_LOCAL_LOCK
#include <atomic>

struct IrredundantRowNode
{
    IrredundantRowNode() :
    next(nullptr), sync(ATOMIC_FLAG_INIT) {};
    
    Row data;
    IrredundantRowNode* next;
    std::atomic_flag sync;
};

#endif

class IrredundantMatrix
{

public:
    IrredundantMatrix(int width);
    void addRow(Row&& row, int* r);
    void addRowConcurrent(Row&& row, int* r);
    void addMatrixConcurrent(IrredundantMatrix&& matrix);
    void clear();
    void printMatrix(std::ostream& stream);
    void printR(std::ostream& stream);

    IrredundantMatrix(IrredundantMatrix& matrix) = delete;
    IrredundantMatrix& operator=(IrredundantMatrix& matrix) = delete;

private:

    void addRowInternal(Row &&row);

#ifdef USE_LOCAL_LOCK
    IrredundantRowNode _head;
    std::atomic_flag _rSync;
#else
    
    std::mutex _rowsMutex;
    std::mutex _rMutex;
    
#ifdef IRREDUNTANT_VECTOR
    std::vector<Row> _rows;
#else
    std::deque<Row> _rows;
#endif

#endif

    int _width;
    std::vector<int> _r;
    
};

#endif // IRREDUNDANTMATRIX_H
