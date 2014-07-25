#include "input_matrix.h"

#include <fstream>
#include <limits>
#include <map>
#include <tuple>
#include <algorithm>

#include "workrow.h"
#include "timecollector.h"

InputMatrix::InputMatrix(std::istream& input)
{
    {
        TimeCollectorEntry reading(ReadingInput);
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
    }

    {
        TimeCollectorEntry preparing(PreparingInput);
        calcR2Matrix();
        sortMatrix();
        calcR2Indexes();
    }
}

InputMatrix::~InputMatrix() {
    delete[] _rMatrix;
    delete[] _r2Matrix;
    delete[] _qMatrix;
}

void InputMatrix::printFeatureMatrix(std::ostream& stream, bool printSize) {
    TimeCollectorEntry writing(WritingOutput);

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
    TimeCollectorEntry writing(WritingOutput);

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
    auto currentId = 0;
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
    _r2Count = currentId;
}

void InputMatrix::sortMatrix() {
    std::vector<int> counts(_r2Count);
    for(auto i=0; i<_rowsCount; ++i) {
        counts[_r2Matrix[i]] += 1;
    }

    std::vector<std::tuple<int, int>> sortedCounts(_r2Count);
    for(auto i=0; i<_r2Count; ++i) {
        sortedCounts[i] = std::make_tuple(i, counts[i]);
    }

    std::sort(sortedCounts.begin(), sortedCounts.end(),
              [](std::tuple<int, int> a, std::tuple<int, int> b){ return std::get<1>(a) > std::get<1>(b); });

    std::vector<int> indexes(_r2Count);
    int currentIndex = 0;
    for(auto i=0; i<_r2Count; ++i) {
        indexes[std::get<0>(sortedCounts[i])] = currentIndex;
        currentIndex += std::get<1>(sortedCounts[i]);
    }

    std::vector<int> newIndexes(_rowsCount);
    for(auto i=0; i<_rowsCount; ++i) {
        newIndexes[i] = indexes[_r2Matrix[i]];
        indexes[_r2Matrix[i]] += 1;
    }

    auto oldQMatrix = _qMatrix;
    auto oldRMatrix = _rMatrix;
    auto oldR2Matrix = _r2Matrix;

    _qMatrix = new int[_rowsCount * _qColsCount];
    _rMatrix = new int[_rowsCount * _rColsCount];
    _r2Matrix = new int[_rowsCount];

    for(auto i=0; i<_rowsCount; ++i) {
        for(auto j=0; j<_qColsCount; ++j) {
            _qMatrix[newIndexes[i]*_qColsCount + j] = oldQMatrix[i*_qColsCount + j];
        }
        for(auto j=0; j<_rColsCount; ++j) {
            _rMatrix[newIndexes[i]*_rColsCount + j] = oldRMatrix[i*_rColsCount + j];
        }
        _r2Matrix[newIndexes[i]] = oldR2Matrix[i];
    }

    delete[] oldQMatrix;
    delete[] oldRMatrix;
    delete[] oldR2Matrix;
}

void InputMatrix::calcR2Indexes() {
    auto currentId = 0;
    auto newId = 0;
    for(auto i=0; i<_rowsCount; ++i) {
        if(_r2Matrix[i] != currentId) {
            _r2indexes.push_back(i);
            currentId = _r2Matrix[i];
            newId += 1;
        }
        _r2Matrix[i] = newId;
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
            TimeCollectorEntry difference(QHandling);
            irredundantMatrix.addRow(Row::createAsDifference(
                                         WorkRow(_qMatrix, offset1+i, _qColsCount), WorkRow(_qMatrix, offset2+j, _qColsCount)));
        }
    }
}
