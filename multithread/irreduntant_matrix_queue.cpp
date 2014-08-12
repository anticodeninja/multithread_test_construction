#include "irredundant_matrix_queue.h"

#include <limits>
#include <utility>

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrixQueue::IrredundantMatrixQueue() {
}

void IrredundantMatrixQueue::addRow(Row &row, bool concurrent) {
    COLLECT_TIME(RMerging);

    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(CrossThreading);
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

void IrredundantMatrixQueue::printMatrix(std::ostream &stream)
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
