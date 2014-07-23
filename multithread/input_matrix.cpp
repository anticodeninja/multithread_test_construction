#include "input_matrix.h"

#include <fstream>
#include <limits>
#include <map>

#include "workrow.h"

InputMatrix::InputMatrix(std::istream& input)
{
    input >> _rowsCount;

    input >> _qColsCount;
    _qMatrix = new int[_rowsCount * _qColsCount];
    for(auto i=0; i<_rowsCount; ++i) {
        for(auto j=0; j<_qColsCount; ++j) {
            setFeature(i, j, parseValue(input));
        }
    }

    input >> _rColsCount;
    _rMatrix = new int[_rowsCount * _rColsCount];
    for(auto i=0; i<_rowsCount; ++i) {
        for(auto j=0; j<_rColsCount; ++j) {
            setImage(i, j, parseValue(input));
        }
    }

    _r2Matrix = new int[_rowsCount];
    calcR2Matrix();
}

InputMatrix::~InputMatrix() {
    delete[] _rMatrix;
    delete[] _r2Matrix;
    delete[] _qMatrix;
}

void InputMatrix::printFeatureMatrix(std::ostream& stream, bool printSize) {
    if(printSize) {
        stream << _rowsCount << " " << _qColsCount << std::endl;
    }

    for(auto i=0; i<_rowsCount; ++i, stream << std::endl) {
        for(auto j=0; j<_qColsCount; ++j, stream << " ") {
            if(getFeature(i, j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << getFeature(i, j);
        }
    }
}

void InputMatrix::printImageMatrix(std::ostream& stream, bool printSize) {
    if(printSize)
        stream << _rowsCount << " " << _rColsCount << std::endl;

    for(auto i=0; i<_rowsCount; ++i, stream << std::endl) {
        for(auto j=0; j<_rColsCount; ++j, stream << " ") {
            if(getImage(i, j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << getImage(i, j);
        }
        stream << "| " << _r2Matrix[i];
    }
}

int InputMatrix::parseValue(std::istream &stream)
{
    std::string buffer;
    stream >> buffer;
    if(buffer == "-")
        return std::numeric_limits<int>::min();
    return stoi(buffer);
}

void InputMatrix::calcR2Matrix()
{
    auto currentId = 1;
    std::map<WorkRow, int> mappings;
    for(auto i=0; i<_rowsCount; ++i) {
        WorkRow currentRow(_rMatrix, i, _rColsCount);
        auto mapping = mappings.find(currentRow);

        if(mapping != mappings.end()) {
            _r2Matrix[i] = mapping->second;
        } else {
            _r2Matrix[i] = currentId;
            mappings[currentRow] = currentId;
            currentId += 1;
        }
    }

    currentId = 0;
    for(auto i=0; i<_rowsCount; ++i) {
        if(_r2Matrix[i] != currentId) {
            _r2indexes.push_back(i);
            std::cout << "|" << _r2indexes[currentId];
            currentId += 1;
        }
    }
}

void InputMatrix::calculateCoverageMatrix(IrredundantMatrix &irredundantMatrix) {
    auto calcLength = [this](size_t x) {
        return (x + 1 < _r2indexes.size())
                ? _r2indexes[x+1] - _r2indexes[x]
                : _rowsCount - _r2indexes[x];
    };

    for(size_t i=0; i<_r2indexes.size()-1; ++i) {
        for(size_t j=i+1; j<_r2indexes.size(); ++j) {
            processBlock(irredundantMatrix, _r2indexes[i], calcLength(i), _r2indexes[j], calcLength(j));
        }
    }
}

void InputMatrix::processBlock(IrredundantMatrix &irredundantMatrix, int offset1, int length1, int offset2, int length2) {
    for(auto i=0; i<length1; ++i) {
        for(auto j=0; j<length2; ++j) {
            irredundantMatrix.addRow(Row::createAsDifference(
                WorkRow(_qMatrix, offset1+i, _qColsCount), WorkRow(_qMatrix, offset2+j, _qColsCount)));
        }
    }
}
