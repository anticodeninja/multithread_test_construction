#include "irredundant_matrix_array.h"

#include <limits>

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrixArray::IrredundantMatrixArray()
{
}

void IrredundantMatrixArray::addRow(Row row, bool concurrent)
{
    COLLECT_TIME(RMerging);

    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(CrossThreading);
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

void IrredundantMatrixArray::printMatrix(std::ostream &stream)
{
    COLLECT_TIME(WritingOutput);

    stream << _rows.size() << " " << (_rows.size() > 0 ? _rows[0].getWidth() : 0) << std::endl;
    for(auto i=_rows.begin(); i !=_rows.end(); ++i, stream << std::endl) {
        for(size_t j=0; j < i->getWidth(); ++j, stream << " ") {
            if(i->getValue(j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << i->getValue(j);
        }
    }
}


