#include "irredundant_matrix.h"

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrix::IrredundantMatrix(int width)
    : _width(width)
{
    _r.resize(width);
}

void IrredundantMatrix::addRow(Row&& row, int* r, bool concurrent)
{
    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }
    
    for(auto i=0; i<_width; ++i) {
        _r[i] += r[i];
    }

    addRowInternal(std::move(row));
}

void IrredundantMatrix::clear(bool concurrent)
{
    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    _rows.clear();
    for(auto i=0; i<_width; ++i) {
        _r[i] = 0;
    }
}

void IrredundantMatrix::addMatrix(IrredundantMatrix &&matrix, bool concurrent)
{
    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    for(auto i=matrix._rows.begin(); i!=matrix._rows.end(); ++i) {
        addRowInternal(std::move(*i));
    }
    
    DEBUG_BLOCK (
       getDebugStream() << "Merge R" << std::endl;
       for(auto i=0; i<matrix._r.size(); ++i) {
           getDebugStream() << _r[i];        
       }

       getDebugStream() << "+" << std::endl;
       for(auto i=0; i<matrix._r.size(); ++i) {
           getDebugStream() << matrix._r[i];        
       }

       getDebugStream() << "=" << std::endl;
       for(auto i=0; i<matrix._r.size(); ++i) {
           getDebugStream() << (matrix._r[i] + _r[i]);
       }
    )

    for(auto i=0; i<matrix._r.size(); ++i) {
        _r[i] += matrix._r[i];
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

void IrredundantMatrix::printMatrix(std::ostream &stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    auto width = getWidth();
    stream << getHeight() << " " << width << std::endl;
    for(auto i=_rows.begin(); i!=_rows.end(); ++i) {
        for(size_t j=0; j < width; ++j, stream << " ") {
            if(i->getValue(j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << i->getValue(j);
        }
        stream << std::endl;
    }
}

void IrredundantMatrix::printR(std::ostream& stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    auto width = getWidth();
    for(size_t i=0; i < width; ++i, stream << " ") {
        stream << _r[i];
    }
}
