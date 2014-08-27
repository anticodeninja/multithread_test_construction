#include "irredundant_matrix_queue.h"

#include <limits>
#include <utility>

#include "global_settings.h"
#include "timecollector.h"

IrredundantMatrixQueue::IrredundantMatrixQueue() {
}

void IrredundantMatrixQueue::addRow(Row&& row, bool concurrent) {
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

