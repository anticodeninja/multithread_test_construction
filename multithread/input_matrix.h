#ifndef INPUTMATRIX_H
#define INPUTMATRIX_H

#include <iostream>
#include <vector>

#include "irredundant_matrix.h"

class InputMatrix
{

public:
    InputMatrix(std::istream& input);
    ~InputMatrix();

    void printFeatureMatrix(std::ostream& stream, bool printSize = false);
    void printImageMatrix(std::ostream& stream, bool printSize = false);
    void calculateCoverageMatrix(IrredundantMatrix& irredundantMatrix);
    void processBlock(IrredundantMatrix &irredundantMatrix, int offset1, int length1, int offset2, int length2);

public:
    inline void setFeature(int i, int j, int value)
    {
        _qMatrix[i*_qColsCount + j] = value;
    }

    inline int getFeature(int i, int j) const
    {
        return _qMatrix[i*_qColsCount + j];
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

private:

    int _rowsCount;
    int _qColsCount;
    int _rColsCount;

    int* _qMatrix;
    int* _rMatrix;
    int* _r2Matrix;

    std::vector<int> _r2indexes;

};

#endif // INPUTMATRIX_H
