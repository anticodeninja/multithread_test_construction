#include "irredundant_matrix_array.h"

#include <limits>

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrixArray::IrredundantMatrixArray()
{
}

void IrredundantMatrixArray::addRow(Row&& row, bool concurrent)
{
    COLLECT_TIME(Timers::RMerging);

    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
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

void IrredundantMatrixArray::enumerate(std::function<void (Row&)> callback)
{
    for(auto i=_rows.begin(); i!=_rows.end(); ++i) {
        callback(*i);
    }
}

int IrredundantMatrixArray::getHeight()
{
    return _rows.size();
}

int IrredundantMatrixArray::getWidth()
{
    return _rows.size() != 0 ? _rows[0].getWidth() : 0;
}


