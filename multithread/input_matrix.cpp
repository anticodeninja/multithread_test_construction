#include "input_matrix.h"

#include <fstream>
#include <limits>
#include <map>
#include <tuple>
#include <algorithm>
#include <thread>

#include "global_settings.h"
#include "workrow.h"
#include "timecollector.h"
#include "fast_plan.h"
#include "irredundant_matrix_array.h"

InputMatrix::InputMatrix(std::istream& input)
{
    {
        COLLECT_TIME(Timers::ReadingInput);
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
        COLLECT_TIME(Timers::PreparingInput);
        calcR2Matrix();
        sortMatrix();
        calcR2Indexes();
        _planBuilder = new FastPlan(_r2Counts.data(), _r2Counts.size());
    }
}

InputMatrix::~InputMatrix() {
    delete[] _rMatrix;
    delete[] _r2Matrix;
    delete[] _qMatrix;
    delete _planBuilder;
}

void InputMatrix::printFeatureMatrix(std::ostream& stream) {
    COLLECT_TIME(Timers::WritingOutput);

    stream << "# FeatureMatrix" << std::endl;
    stream << _rowsCount << " " << _qColsCount << std::endl;

    for(auto i=0; i<_rowsCount; ++i, stream << std::endl) {
        for(auto j=0; j<_qColsCount; ++j, stream << " ") {
            if(getFeature(i, j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << getFeature(i, j);
        }
    }
}

void InputMatrix::printImageMatrix(std::ostream& stream) {
    COLLECT_TIME(Timers::WritingOutput)

    stream << "# ImageMatrix" << std::endl;
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

void InputMatrix::printDebugInfo(std::ostream& stream) {
    COLLECT_TIME(Timers::WritingOutput)

    stream << "# R2Indexes" << std::endl;
    for(size_t i=0; i<_r2Counts.size(); ++i) {
        stream << _r2Indexes[i] << " - " << _r2Counts[i] << std::endl;
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

void InputMatrix::calcR2Indexes() {
    auto currentId = 0;
    auto newId = 0;
    auto startIndex = 0;

    _r2Indexes.push_back(0);
    for(auto i=0; i<_rowsCount; ++i) {
        if(_r2Matrix[i] != currentId) {
            _r2Indexes.push_back(i);
            _r2Counts.push_back(i - startIndex);

            currentId = _r2Matrix[i];
            startIndex = i;
            newId += 1;
        }
        _r2Matrix[i] = newId;
    }
    _r2Counts.push_back(_rowsCount - startIndex);
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

void InputMatrix::calculateSingleThread(IrredundantMatrixBase &irredundantMatrix) {
    for(size_t i=0; i<_r2Indexes.size()-1; ++i) {
        for(size_t j=i+1; j<_r2Indexes.size(); ++j) {
            std::unique_ptr<IrredundantMatrixArray> matrixForThread;
            IrredundantMatrixBase *currentMatrix;
            #ifdef DIFFERENT_MATRICES
                matrixForThread = std::unique_ptr<IrredundantMatrixArray>(new IrredundantMatrixArray());
                currentMatrix = &*matrixForThread;
            #else
                currentMatrix = &irredundantMatrix;
            #endif

            processBlock(*currentMatrix, _r2Indexes[i], _r2Counts[i], _r2Indexes[j], _r2Counts[j], false);

            #ifdef DIFFERENT_MATRICES
                irredundantMatrix.addMatrix(std::move(*matrixForThread), false);
            #endif
        }
    }
}

void InputMatrix::calculateMultiThreadWithOptimalPlanBuilding(IrredundantMatrixBase &irredundantMatrix)
{
    auto threadRang = 1;
    auto indexesCount = 0;
    for(; _r2Count / 2 > 1 << (threadRang - 1); ++threadRang);
    for(auto i=0; i<=threadRang; ++i) indexesCount += 1 << i;
    auto maxThreads = 1 << threadRang;

    DEBUG_INFO("ThreadRang: " << threadRang);
    DEBUG_INFO("IndexesCount: " << indexesCount);
    DEBUG_INFO("MaxThreads: " << maxThreads);

    std::vector<int> indexes(3 * indexesCount);
    std::vector<std::unique_ptr<IrredundantMatrixBase>> threadIrredunantMatrices(maxThreads);
    std::vector<std::thread> threads(maxThreads);

    auto setBegin = [&indexes](int id, int value) {indexes[3*id + 0] = value;};
    auto getBegin = [&indexes](int id) {return indexes[3*id + 0];};
    auto setMedian = [&indexes](int id, int value) {indexes[3*id + 1] = value;};
    auto getMedian = [&indexes](int id) {return indexes[3*id + 1];};
    auto setEnd = [&indexes](int id, int value) {indexes[3*id + 2] = value;};
    auto getEnd = [&indexes](int id) {return indexes[3*id + 2];};

    setBegin(0, 0);
    setEnd(0, _r2Count - 1);

    auto stepBaseIndex = 0;
    for(auto rang=0; rang<=threadRang; ++rang) {
        auto threadForStep = 1 << rang;
        DEBUG_INFO("Step: " << (rang+1) << ", ThreadForStep: " << threadForStep);

        for(auto j=0; j<threadForStep; ++j) {
            auto id = stepBaseIndex + j;
            COLLECT_TIME(Timers::Threading);
            threads[j] = std::thread([this, rang, id, threadRang,
                                     &irredundantMatrix, &indexes,
                                     &setBegin, &setMedian, &setEnd,
                                     &getBegin, &getMedian, &getEnd](){
                std::unique_ptr<IrredundantMatrixArray> matrixForThread;
                IrredundantMatrixBase *currentMatrix;
                #ifdef DIFFERENT_MATRICES
                    matrixForThread = std::unique_ptr<IrredundantMatrixArray>(new IrredundantMatrixArray());
                    currentMatrix = &*matrixForThread;
                #else
                    currentMatrix = &irredundantMatrix;
                #endif

                DEBUG_INFO("Work on step: " << id <<
                           ", begin: " << getBegin(id) << ", end: " << getEnd(id));

                if(getBegin(id) != getEnd(id)) {
                    {
                        COLLECT_TIME(Timers::PlanBuilding);
                        setMedian(id, _planBuilder->FindNextStep(getBegin(id), getEnd(id)));
                    }

                    for(auto i=getBegin(id); i<=getMedian(id); ++i) {
                        for(auto j=getMedian(id)+1; j<=getEnd(id); ++j) {
                            processBlock(*currentMatrix,
                                         _r2Indexes[_planBuilder->GetIndex(i)], _r2Counts[_planBuilder->GetIndex(i)],
                                         _r2Indexes[_planBuilder->GetIndex(j)], _r2Counts[_planBuilder->GetIndex(j)],
                                    true);
                        }
                    }

                    if(rang != threadRang) {
                        setBegin(2*id + 1, getBegin(id));
                        setEnd(2*id + 1, getMedian(id));
                        setBegin(2*id + 2, getMedian(id) + 1);
                        setEnd(2*id + 2, getEnd(id));
                    }
                }

                #ifdef DIFFERENT_MATRICES
                    irredundantMatrix.addMatrix(std::move(*matrixForThread), true);
                #endif
            });
        }
        for(auto j=0; j<threadForStep; ++j) {
            threads[j].join();
        }

        stepBaseIndex += threadForStep;
    }
}

void InputMatrix::processBlock(IrredundantMatrixBase &irredundantMatrix,
                               int offset1, int length1, int offset2, int length2, bool concurrent) {
    for(auto i=0; i<length1; ++i) {
        for(auto j=0; j<length2; ++j) {
            COLLECT_TIME(Timers::QHandling);
            irredundantMatrix.addRow(Row::createAsDifference(
                                         WorkRow(_qMatrix, offset1+i, _qColsCount),
                                         WorkRow(_qMatrix, offset2+j, _qColsCount)), concurrent);
        }
    }
}
