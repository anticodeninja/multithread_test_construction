#include "irredundant_matrix.hpp"

#include <thread>

#include "global_settings.h"
#include "timecollector.hpp"

IrredundantMatrix::IrredundantMatrix(int width)
    : _width(width)
#ifdef USE_LOCAL_LOCK
    , _rSync(ATOMIC_FLAG_INIT)
#endif
{
    _r.resize(width);
}


void IrredundantMatrix::addRow(Row&& row, int* r)
{
    for(auto i=0; i<_width; ++i) {
        _r[i] += r[i];
    }

    addRowInternal(std::move(row));
}

#ifdef USE_LOCAL_LOCK

void IrredundantMatrix::addRowConcurrent(Row&& row, int* r)
{
    START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
    while (_rSync.test_and_set(std::memory_order_acquire));
    STOP_COLLECT_TIME(crossThreading);

    for(auto i=0; i<_width; ++i) {
        _r[i] += r[i];
    }
    _rSync.clear(std::memory_order_release);

    addRowInternal(std::move(row));
}

void IrredundantMatrix::addMatrixConcurrent(IrredundantMatrix &&matrix)
{
    START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
    while (_rSync.test_and_set(std::memory_order_acquire));
    STOP_COLLECT_TIME(crossThreading);

    for(auto i=0; i<_width; ++i) {
        _r[i] += matrix._r[i];
    }
    _rSync.clear(std::memory_order_release);

    for(auto current = matrix._head.next; current != nullptr; current = current->next) {
        addRowInternal(std::move(current->data));
    }
}

void IrredundantMatrix::addRowInternal(Row &&row) {
    START_COLLECT_TIME(rMerging, Counters::RMerging);

    IrredundantRowNode* start = nullptr;
    auto ageBoundMin = 0;
    auto ageBoundMax = 0;

    for (;;) {
        START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
        while (_head.sync.test_and_set(std::memory_order_acquire));
        PAUSE_COLLECT_TIME(crossThreading);

        auto prev = &_head;
        start = _head.next;
        ageBoundMax = _head.age;

        for(;;) {
            auto current = prev->next;

            if (current == nullptr || current->age < ageBoundMin) {
                prev->sync.clear();
                if(current != nullptr) {
                    DEBUG_INFO("-EX " << current->age << " < " << ageBoundMin);
                }
                break;
            } else {
                CONTINUE_COLLECT_TIME(crossThreading);
                while (current->sync.test_and_set(std::memory_order_acquire));
                PAUSE_COLLECT_TIME(crossThreading);

                if (current->data.isInclude(row)) {
                    DEBUG_INFO("-CB " << row << " | " << current->data);
                    prev->sync.clear(std::memory_order_relaxed);
                    current->sync.clear(std::memory_order_release);
                    return;
                } else if (row.isInclude(current->data)) {
                    DEBUG_INFO("-CE " << row << " | " << current->data);
                    prev->next = current->next;
                    current->sync.clear(std::memory_order_release);
                    delete current;
                } else {
                    auto oldPrev = prev;
                    prev = current;
                    oldPrev->sync.clear(std::memory_order_release);
                }
            }
        }

        // Append new row
        CONTINUE_COLLECT_TIME(crossThreading);
        while (_head.sync.test_and_set(std::memory_order_acquire));
        STOP_COLLECT_TIME(crossThreading);

        if (_head.next == start) {
            auto newNode = new IrredundantRowNode();
            newNode->data = std::move(row);
            newNode->next = _head.next;
            newNode->age = ++_head.age;
            _head.next = newNode;

            DEBUG_INFO("-AR " << newNode->data << ", " << newNode->age);

            _head.sync.clear(std::memory_order_release);
            return;
        } else {
            ageBoundMin = ageBoundMax;
            _head.sync.clear(std::memory_order_release);
            std::this_thread::yield();
        }
    }

    STOP_COLLECT_TIME(rMerging);
}

void IrredundantMatrix::clear()
{
    for(auto i=0; i<_width; ++i) {
        _r[i] = 0;
    }

    auto prev = _head.next;
    _head.next = nullptr;

    while (prev != nullptr) {
        auto next = prev->next;
        delete prev;
        prev = next;
    }
}

void IrredundantMatrix::fill(DataFile& datafile)
{
    auto height = 0;
    for(auto current = _head.next; current != nullptr; current = current->next) {
        height += 1;
    }

    auto i = 0;
    auto uim = new feature_t[height * _width];
    for(auto current = _head.next; current != nullptr; current = current->next) {
        for(auto j = 0; j < _width; ++j) {
            uim[i * _width + j] = current->data.getValue(j);
        }
        i += 1;
    }

    auto uimWeights = new feature_t[_width];
    for(size_t j = 0; j < _width; ++j) {
        uimWeights[j] = _r[j];
    }

    datafile.setUimBlock(uim, height, _width);
    datafile.setUimWeightsBlock(uimWeights, _width);
}

#else

void IrredundantMatrix::addRowConcurrent(Row&& row, int* r)
{
    START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
    _rMutex.lock();
    PAUSE_COLLECT_TIME(crossThreading);

    for(auto i=0; i<_width; ++i) {
        _r[i] += r[i];
    }

    _rMutex.unlock();

    CONTINUE_COLLECT_TIME(crossThreading);
    _rowsMutex.lock();
    STOP_COLLECT_TIME(crossThreading);

    addRowInternal(std::move(row));

    _rowsMutex.unlock();
}

void IrredundantMatrix::addMatrixConcurrent(IrredundantMatrix &&matrix)
{
    START_COLLECT_TIME(crossThreading, Counters::CrossThreading);
    _rMutex.lock();
    PAUSE_COLLECT_TIME(crossThreading);

    for(auto i=0; i<_width; ++i) {
        _r[i] += matrix._r[i];
    }

    _rMutex.unlock();

    CONTINUE_COLLECT_TIME(crossThreading);
    _rowsMutex.lock();
    STOP_COLLECT_TIME(crossThreading);

    for(auto i=matrix._rows.begin(); i!=matrix._rows.end(); ++i) {
        addRowInternal(std::move(*i));
    }

    _rowsMutex.unlock();
}

void IrredundantMatrix::clear()
{
    for(auto i=0; i<_width; ++i) {
        _r[i] = 0;
    }

    _rows.clear();
}

void IrredundantMatrix::fill(DataFile& datafile)
{
    auto uim = new feature_t[_rows.size() * _width];
    for(auto i = 0; i < _rows.size(); ++i) {
        for(auto j = 0; j < _width; ++j) {
            uim[i * _width + j] = _rows[i].getValue(j);
        }
    }

    auto uimWeights = new feature_t[_width];
    for(size_t j = 0; j < _width; ++j) {
        uimWeights[j] = _r[j];
    }

    datafile.setUimBlock(uim, _rows.size(), _width);
    datafile.setUimWeightsBlock(uimWeights, _width);
}

#ifdef IRREDUNTANT_VECTOR

void IrredundantMatrix::addRowInternal(Row &&row) {
    START_COLLECT_TIME(rMerging, Counters::RMerging);

    auto i = 0;
    while(i < _rows.size()) {
        if(_rows[i].isInclude(row)) {
            DEBUG_INFO("-CB " << row << " | " << *i);
            return;
        }
        if(row.isInclude(_rows[i])) {
            if(i != (_rows.size() - 1)) {
                DEBUG_INFO("-CE " << row << " | " << *i);
                _rows[i] = std::move(_rows[_rows.size() - 1]);
            }
            _rows.pop_back();
            continue;
        }
        ++i;
    }

    DEBUG_INFO("-AR " << row);
    _rows.push_back(std::move(row));

    STOP_COLLECT_TIME(rMerging);
}

#else

void IrredundantMatrix::addRowInternal(Row &&row) {
    START_COLLECT_TIME(rMerging, Counters::RMerging);

    auto i = _rows.begin();
    while(i != _rows.end()) {
        if(i->isInclude(row)) {
            DEBUG_INFO("-CB " << row << " | " << *i);
            return;
        }
        if(row.isInclude(*i)) {
            DEBUG_INFO("-CE " << row << " | " << *i);
            i = _rows.erase(i);
            continue;
        }
        ++i;
    }

    DEBUG_INFO("-AR " << row);
    _rows.push_front(std::move(row));

    STOP_COLLECT_TIME(rMerging);
}

#endif

#endif
