#include "irredundant_matrix.h"

#include <limits>

#include "timecollector.h"

IrredundantMatrix::IrredundantMatrix() {
}

void IrredundantMatrix::addRow(const Row &row) {
    TimeCollectorEntry merging(RMerging);

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

    _rows.push_back(row);
}

void IrredundantMatrix::printMatrix(std::ostream &stream, bool printSize)
{
    TimeCollectorEntry writing(WritingOutput);

    if(printSize)
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
