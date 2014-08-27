#include "irredundant_matrix_base.h"

#include <iostream>

#include "global_settings.h"
#include "timecollector.h"

void IrredundantMatrixBase::addMatrix(IrredundantMatrixBase &&matrix, bool concurrent)
{
    std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
    if(concurrent) {
        COLLECT_TIME(Timers::CrossThreading);
        lock.lock();
    }

    matrix.enumerate([this](Row& row) {
        addRow(std::move(row), false);
    });
}

void IrredundantMatrixBase::printMatrix(std::ostream &stream)
{
    COLLECT_TIME(Timers::WritingOutput);

    auto width = getWidth();
    stream << getHeight() << " " << width << std::endl;
    enumerate([&stream, &width](Row& row) {
        for(size_t j=0; j < width; ++j, stream << " ") {
            if(row.getValue(j) == std::numeric_limits<int>::min())
                stream << '-';
            else
                stream << row.getValue(j);
        }
        stream << std::endl;
    });
}
