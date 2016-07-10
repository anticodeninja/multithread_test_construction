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
#include "irredundant_matrix.h"

#ifdef MULTITHREAD_DIVIDE2
#include "divide2_plan.h"
#elif MULTITHREAD_MASTERWORKER
#include "masterworker_plan.h"
#endif

#ifdef MULTITHREAD
#ifndef DIFFERENT_MATRICES
#define ADD_ROW_CONCURRENT
#endif
#endif

InputMatrix::InputMatrix(std::istream& input)
{
    START_COLLECT_TIME(readingInput, Counters::ReadingInput);
    input >> _rowsCount;

    input >> _qColsCount;
    _qMatrix = new int[_rowsCount * _qColsCount];
    _qMinimum = new int[_qColsCount];
    _qMaximum = new int[_qColsCount];
    for(auto j=0; j<_qColsCount; ++j) {
        _qMinimum[j] = DASH;
        _qMaximum[j] = DASH;
    }
    for(auto i=0; i<_rowsCount; ++i) {
        for(auto j=0; j<_qColsCount; ++j) {
            auto value = parseValue(input);
            setFeature(i, j, value);
            if (value != DASH) {
                if (_qMinimum[j] == DASH || value < _qMinimum[j])
                    _qMinimum[j] = value;
                if (_qMaximum[j] == DASH || _qMaximum[j] < value)
                    _qMaximum[j] = value;
            }
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
    STOP_COLLECT_TIME(readingInput);

    START_COLLECT_TIME(preparingInput, Counters::PreparingInput);
    calcR2Matrix();
    sortMatrix();
    calcR2Indexes();
    STOP_COLLECT_TIME(preparingInput);
}

InputMatrix::~InputMatrix() {
    delete[] _rMatrix;
    delete[] _r2Matrix;
    delete[] _qMatrix;
    delete[] _qMaximum;
    delete[] _qMinimum;
}

void InputMatrix::printFeatureMatrix(std::ostream& stream) {
    START_COLLECT_TIME(writingOutput, Counters::WritingOutput);

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

    STOP_COLLECT_TIME(writingOutput);
}

void InputMatrix::printImageMatrix(std::ostream& stream) {
    START_COLLECT_TIME(writingOutput, Counters::WritingOutput);

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

    STOP_COLLECT_TIME(writingOutput);
}

void InputMatrix::printDebugInfo(std::ostream& stream) {
    START_COLLECT_TIME(writingOutput, Counters::WritingOutput);

    stream << "qMinimum" << std::endl;
    for(size_t i=0; i<_qColsCount; ++i) {
        stream << _qMinimum[i] << " ";
    }
    stream << std::endl;

    stream << "qMaximum" << std::endl;
    for(size_t i=0; i<_qColsCount; ++i) {
        stream << _qMaximum[i] << " ";
    }
    stream << std::endl;
    
    stream << "# R2Indexes" << std::endl;
    for(size_t i=0; i<_r2Counts.size(); ++i) {
        stream << _r2Indexes[i] << " - " << _r2Counts[i] << std::endl;
    }

    STOP_COLLECT_TIME(writingOutput);
}

int InputMatrix::parseValue(std::istream &stream)
{
    std::string buffer;
    stream >> buffer;
    if(buffer == "-")
        return DASH;
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
    auto newId = 0;
    auto startIndex = 0;

    auto currentId = _r2Matrix[0];
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

#ifdef MULTITHREAD_DIVIDE2

void InputMatrix::calculate(IrredundantMatrix &irredundantMatrix)
{
    Divide2Plan planBuilder(_r2Counts.data(), _r2Counts.size());
    std::vector<std::thread> threads(planBuilder.getMaxThreadsCount());

    for(auto step = 0; step < planBuilder.getStepsCount(); ++step) {
        DEBUG_INFO("Step: " << step);

        for(auto threadId = 0; threadId < planBuilder.getThreadsCountForStep(step); ++threadId) {
            START_COLLECT_TIME(threading, Counters::Threading);
            threads[threadId] = std::thread([this, step, threadId, &irredundantMatrix, &planBuilder]()
            {
                TimeCollector::ThreadInitialize();
                
                #ifdef DIFFERENT_MATRICES
                IrredundantMatrix matrixForThread(_qColsCount);
                auto currentMatrix = &matrixForThread;
                #else
                auto currentMatrix = &irredundantMatrix;
                #endif

                auto task = planBuilder.getTask(step, threadId);

                if (task->isEmpty()) {
                    DEBUG_INFO("Thread " << threadId << " stopped");
                    return;
                }

                DEBUG_INFO("Thread " << threadId << " is working on " <<
                           task->getFirstSize() << ":" << task->getSecondSize());

                for(auto i=0; i<task->getFirstSize(); ++i) {
                    for(auto j=0; j<task->getSecondSize(); ++j) {
                        processBlock(*currentMatrix,
                                     _r2Indexes[task->getFirst(i)], _r2Counts[task->getFirst(i)],
                                     _r2Indexes[task->getSecond(j)], _r2Counts[task->getSecond(j)]);
                    }
                }

                #ifdef DIFFERENT_MATRICES
                irredundantMatrix.addMatrixConcurrent(std::move(matrixForThread));
                #endif

                TimeCollector::ThreadFinalize();
            });
            STOP_COLLECT_TIME(threading);
        }
        
        for(auto threadId = 0; threadId < planBuilder.getThreadsCountForStep(step); ++threadId) {
            threads[threadId].join();
        }
    }
}

#elif MULTITHREAD_MASTERWORKER

void InputMatrix::calculate(IrredundantMatrix &irredundantMatrix)
{
    auto maxThreads = std::thread::hardware_concurrency();;

    DEBUG_INFO("MaxThreads: " << maxThreads);

    std::vector<std::thread> threads(maxThreads);

    MasterWorkerPlan planBuilder(_r2Counts.data(), _r2Counts.size());
    
    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([this, threadId, &irredundantMatrix, &planBuilder]()
        {
            TimeCollector::ThreadInitialize();
            
            #ifdef DIFFERENT_MATRICES
            IrredundantMatrix matrixForThread(_qColsCount);
            auto currentMatrix = &matrixForThread;
            #else
            auto currentMatrix = &irredundantMatrix;
            #endif

            DEBUG_INFO("Thread " << threadId << " started");
            
            for(;;) {
                auto task = planBuilder.getTask();
                if (task->isEmpty()) {
                    DEBUG_INFO("Thread " << threadId << " stopped");
                    return;
                }

                DEBUG_INFO("Thread " << threadId << " is working on " << task->getFirst() << ":" << task->getSecond());

                #ifdef DIFFERENT_MATRICES
                matrixForThread.clear();
                #endif
 
                processBlock(*currentMatrix,
                             _r2Indexes[task->getFirst()], _r2Counts[task->getFirst()],
                             _r2Indexes[task->getSecond()], _r2Counts[task->getSecond()]);

                #ifdef DIFFERENT_MATRICES
                irredundantMatrix.addMatrixConcurrent(std::move(matrixForThread));
                #endif
            }

            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }
    
    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        threads[threadId].join();
    }
}
    
#else

void InputMatrix::calculate(IrredundantMatrix &irredundantMatrix) {
    #ifdef DIFFERENT_MATRICES
    IrredundantMatrix matrixForThread IrredundantMatrix(getFeatureWidth());
    auto currentMatrix = &matrixForThread;
    #else
    auto currentMatrix = &irredundantMatrix;
    #endif
            
    for(size_t i=0; i<_r2Indexes.size()-1; ++i) {
        for(size_t j=i+1; j<_r2Indexes.size(); ++j) {
            #ifdef DIFFERENT_MATRICES
            matrixForThread.clear()
            #endif

            processBlock(*currentMatrix, _r2Indexes[i], _r2Counts[i], _r2Indexes[j], _r2Counts[j]);

            #ifdef DIFFERENT_MATRICES
            irredundantMatrix.addMatrixConcurrent(std::move(matrixForThread));
            #endif
        }
    }
}
    
#endif

void InputMatrix::processBlock(IrredundantMatrix &irredundantMatrix,
                               int offset1, int length1, int offset2, int length2) {
    for(auto i=0; i<length1; ++i) {
        for(auto j=0; j<length2; ++j) {
            START_COLLECT_TIME(qHandling, Counters::QHandling);

            auto diffRow = Row::createAsDifference(
                                                   WorkRow(_qMatrix, offset1+i, _qColsCount),
                                                   WorkRow(_qMatrix, offset2+j, _qColsCount));

            int r[_qColsCount];
            calcRVector(r, offset1+i, offset2+j);
            STOP_COLLECT_TIME(qHandling);

            #ifdef ADD_ROW_CONCURRENT
            irredundantMatrix.addRowConcurrent(std::move(diffRow), r);
            #else
            irredundantMatrix.addRow(std::move(diffRow), r);
            #endif
        }
    }
}

void InputMatrix::calcRVector(int* r, int row1, int row2) {
    auto multiplier1 = 1;
    auto multiplier2 = 1;            

    for(auto k=0; k<_qColsCount; ++k) {
        r[k] = 0;
        if(getFeature(row1, k) == DASH) {
            multiplier1 *= getFeatureValuesCount(k);
        }
        if(getFeature(row2, k) == DASH) {
            multiplier2 *= getFeatureValuesCount(k);
        }
    }

    auto calcLimits = [this](int row, int col) {
        return getFeature(row, col) == DASH
           ? std::tuple<int, int>(_qMinimum[col], _qMaximum[col])
           : std::tuple<int, int>(getFeature(row, col), getFeature(row, col));
    };
    
    for (auto k=0; k<_qColsCount; ++k) {
        auto multiplier = multiplier1 * multiplier2;
        if(getFeature(row1, k) == DASH) {
            multiplier /= getFeatureValuesCount(k);
        }
        if(getFeature(row2, k) == DASH) {
            multiplier /= getFeatureValuesCount(k);
        }
        
        auto limit1 = calcLimits(row1, k);
        for (auto i = std::get<0>(limit1); i <= std::get<1>(limit1); ++i) {
            auto limit2 = calcLimits(row2, k);
            for (auto j = std::get<0>(limit2); j <= std::get<1>(limit2); ++j) {
                r[k] += std::abs(i - j) * multiplier;
            }
        }
    }
}
