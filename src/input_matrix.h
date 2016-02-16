#ifndef INPUTMATRIX_H
#define INPUTMATRIX_H

#include <iostream>
#include <vector>

#include "irredundant_matrix.h"

class InputMatrix
{

public:
    const int DASH = std::numeric_limits<int>::min();
    
    InputMatrix(std::istream& input);
    ~InputMatrix();

    void printFeatureMatrix(std::ostream& stream);
    void printImageMatrix(std::ostream& stream);
    void printDebugInfo(std::ostream &stream);

    void processBlock(IrredundantMatrix &irredundantMatrix,
                      int offset1, int length1, int offset2, int length2);

    void calculate(IrredundantMatrix& irredundantMatrix);

public:
    inline void setFeature(int i, int j, int value)
    {
        _qMatrix[i*_qColsCount + j] = value;
    }

    inline int getFeature(int i, int j) const
    {
        return _qMatrix[i*_qColsCount + j];
    }

    inline int getFeatureValuesCount(int j) const
    {
        return _qMaximum[j] - _qMinimum[j] + 1;
    }

    inline int getFeatureWidth() const
    {
        return _qColsCount;
    }

    inline void setImage(int i, int j, int value)
    {
        _rMatrix[i*_rColsCount + j] = value;
    }

    inline int getImage(int i, int j) const
    {
        return _rMatrix[i*_rColsCount + j];
    }

private:

    int parseValue(std::istream& stream);
    void calcR2Matrix();
    void sortMatrix();
    void calcR2Indexes();
    void calcRVector(int* r, int row1, int row2);

    void calcUseSingleThreadAlgo(IrredundantMatrix& irredundantMatrix);
    void calcUseMultithreadDivide2Algo(IrredundantMatrix& irredundantMatrix);
    void calcUseMultithreadMasterWorkerAlgo(IrredundantMatrix& irredundantMatrix);

private:

    int _rowsCount;
    int _qColsCount;
    int _rColsCount;

    int* _qMatrix;
    int* _qMinimum;
    int* _qMaximum;
    int* _rMatrix;

    int* _r2Matrix;
    int _r2Count;

    std::vector<int> _r2Indexes;
    std::vector<int> _r2Counts;
};

#endif // INPUTMATRIX_H>
