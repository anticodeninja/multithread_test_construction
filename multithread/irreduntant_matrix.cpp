#include "irredundant_matrix.h"

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrix::IrredundantMatrix() {
}

#ifdef IRREDUNTANT_VECTOR
void IrredundantMatrix::addRow(Row&& row, bool concurrent)
{
    COLLECT_TIME(Timers::RMerging);

    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {D
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    auto i = 0;
    while(i < _rows.size()) {
        if(_rows[i].isInclude(row)) {
            return;
        }
        if(row.isInclude(_rows[i])) {
            if(i != (_rows.size() - 1)) {
                _rows[i] = std::move(_rows[_rows.size() - 1]);
            }
            _rows.pop_back();
            continue;
        }
        ++i;
    }

    _rows.push_back(std::move(row));
}
#else
void IrredundantMatrix::addRow(Row&& row, bool concurrent) {
    COLLECT_TIME(Timers::RMerging);

    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    auto i = _rows.begin();
    while(i != _rows.end()) {
        if(i->isInclude(row)) {
            return;
        }
        if(row.isInclude(*i)) {
            i = _rows.erase(i);
            continue;
        }
        ++i;
    }

    _rows.push_back(std::move(row));
}
#endif

void IrredundantMatrix::addMatrix(IrredundantMatrix &&matrix, bool concurrent)
{
    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    for(auto i=_rows.begin(); i!=_rows.end(); ++i) {
        addRow(std::move(*i), false);
    }
}

int IrredundantMatrix::getHeight()
{
    return _rows.size();
}

int IrredundantMatrix::getWidth()
{
    return _rows.size() != 0 ? _rows[0].getWidth() : 0;
}

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