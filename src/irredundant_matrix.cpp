#include "irredundant_matrix.h"

#include <thread>

#include "global_settings.h"
#include "timecollector.h"

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
    {
        COLLECT_TIME(Timers::CrossThreading);
        while (_rSync.test_and_set(std::memory_order_acquire));
        for(auto i=0; i<_width; ++i) {
            _r[i] += r[i];
        }
        _rSync.clear(std::memory_order_release);  
    }

    addRowInternal(std::move(row));
}

void IrredundantMatrix::addMatrixConcurrent(IrredundantMatrix &&matrix)
{
    {
        COLLECT_TIME(Timers::CrossThreading);
        while (_rSync.test_and_set(std::memory_order_acquire));
        for(auto i=0; i<_width; ++i) {
            _r[i] += matrix._r[i];
        }
        _rSync.clear(std::memory_order_release);  
    }

    for(auto current = _head.next; current != nullptr; current = current->next) {
        addRowInternal(std::move(current->data));    
    }
}

void IrredundantMatrix::addRowInternal(Row &&row) {
    COLLECT_TIME(Timers::RMerging);

    IrredundantRowNode* start = nullptr;
    IrredundantRowNode* prevStart = nullptr;

    for (;;) {
        while (_head.sync.test_and_set(std::memory_order_acquire));
        
        auto prev = &_head;
        start = _head.next;

        for(;;) {
            auto current = prev->next;
            
            if (current == nullptr || current == prevStart) {
                prev->sync.clear();
                break;
            } else {
                while (current->sync.test_and_set(std::memory_order_acquire));
                
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
        while (_head.sync.test_and_set(std::memory_order_acquire));
            
        if (_head.next == start) {
            DEBUG_INFO("-AR " << row);
            auto newNode = new IrredundantRowNode();
            newNode->data = std::move(row);
            newNode->next = _head.next;
            _head.next = newNode;
            
            _head.sync.clear(std::memory_order_release);
            return;
        } else {
            _head.sync.clear(std::memory_order_release);
            prevStart = start;
            std::this_thread::yield();
        }
    }
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

void IrredundantMatrix::printMatrix(std::ostream &stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    auto height = 0;
    for(auto current = _head.next; current != nullptr; current = current->next) {
        height += 1;
    }

    stream << height << " " << _width << std::endl;

    for(auto current = _head.next; current != nullptr; current = current->next) {
        for(size_t j=0; j < _width; ++j, stream << " ") {
            if(current->data.getValue(j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << current->data.getValue(j);
        }
        stream << std::endl;
    }
}

#else

void IrredundantMatrix::addRowConcurrent(Row&& row, int* r)
{
    {
        COLLECT_TIME(Timers::CrossThreading);
        std::unique_lock<std::mutex> rLock(_rMutex);
        for(auto i=0; i<_width; ++i) {
            _r[i] += r[i];
        }
    }
    
    std::unique_lock<std::mutex> rowsLock(_rowsMutex, std::defer_lock);
    {
        COLLECT_TIME(Timers::CrossThreading);
        rowsLock.lock();
    }

    addRowInternal(std::move(row));
}

void IrredundantMatrix::addMatrixConcurrent(IrredundantMatrix &&matrix)
{
    {
        COLLECT_TIME(Timers::CrossThreading);
        std::unique_lock<std::mutex> rLock(_rMutex);
        for(auto i=0; i<_width; ++i) {
            _r[i] += matrix._r[i];
        }
    }
    
    std::unique_lock<std::mutex> rowsLock(_rowsMutex, std::defer_lock);
    {
        COLLECT_TIME(Timers::CrossThreading);
        rowsLock.lock();
    }

    for(auto i=matrix._rows.begin(); i!=matrix._rows.end(); ++i) {
        addRowInternal(std::move(*i));
    }
}

void IrredundantMatrix::clear()
{
    for(auto i=0; i<_width; ++i) {
        _r[i] = 0;
    }
        
    _rows.clear();
}

void IrredundantMatrix::printMatrix(std::ostream &stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    stream << _rows.size() << " " << _width << std::endl;
    for(auto i=_rows.begin(); i != _rows.end(); ++i) {
        for(size_t j=0; j < _width; ++j, stream << " ") {
            if(i->getValue(j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << i->getValue(j);
        }
        stream << std::endl;
    }
}

#ifdef IRREDUNTANT_VECTOR

void IrredundantMatrix::addRowInternal(Row &&row) {
    COLLECT_TIME(Timers::RMerging);
    
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
}

#else

void IrredundantMatrix::addRowInternal(Row &&row) {
    COLLECT_TIME(Timers::RMerging);
    
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
    _rows.push_back(std::move(row));
}

#endif

#endif

void IrredundantMatrix::printR(std::ostream& stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    for(size_t i=0; i < _width; ++i, stream << " ") {
        stream << _r[i];
    }
}
