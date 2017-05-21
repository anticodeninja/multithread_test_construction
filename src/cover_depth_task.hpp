#ifndef COVER_DEPTH_TASK_H
#define COVER_DEPTH_TASK_H

class CoverDepthTask {
 public:
    CoverDepthTask(uint_fast16_t column,
                   uint_fast16_t featuresCount,
                   uint_fast8_t* mask,
                   uint_fast32_t rowsCount,
                   uint_fast8_t** rows,
                   uint_fast8_t* covering) {
        _column = column;

        _mask = new uint_fast8_t[featuresCount];
        for(uint_fast16_t i=0; i<featuresCount; ++i) {
            _mask[i] = mask[i];
        }

        _rowsCount = rowsCount;
        _rows = new uint_fast8_t*[rowsCount];
        _covering = new uint_fast8_t[rowsCount];
        for(uint_fast16_t i=0; i<rowsCount; ++i) {
            _rows[i] = rows[i];
            _covering[i] = covering[i];
        }
    }

    static CoverDepthTask* createInitTask(uint_fast16_t column,
                                          uint_fast8_t* uim,
                                          uint_fast32_t rowsCount,
                                          uint_fast16_t featuresCount) {
        uint_fast8_t mask[featuresCount];
        for(uint_fast16_t i=0; i<featuresCount; ++i) {
            mask[i] = 0;
        }

        uint_fast8_t* rows[rowsCount];
        uint_fast8_t covering[rowsCount];
        for(uint_fast16_t i=0; i<rowsCount; ++i) {
            rows[i] = &uim[i * featuresCount];
            covering[i] = 0;
        }

        return new CoverDepthTask(column, featuresCount, mask, rowsCount, rows, covering);
    }

    virtual ~CoverDepthTask() {
        delete [] _mask;
        delete [] _rows;
        delete [] _covering;
    }

    CoverDepthTask(const CoverDepthTask& result) = delete;
    CoverDepthTask& operator=(const CoverDepthTask&) = delete;

    inline uint_fast16_t getColumn() const { return _column; }
    inline uint_fast8_t* getMask() const { return _mask; }

    inline uint_fast32_t getRowsCount() const { return _rowsCount; }
    inline uint_fast8_t** getRows() const { return _rows; }
    inline uint_fast8_t* getCovering() const { return _covering; }

private:
    uint_fast16_t _column;
    uint_fast8_t* _mask;

    uint_fast32_t _rowsCount;
    uint_fast8_t** _rows;
    uint_fast8_t* _covering;
};

#endif // COVER_DEPTH_TASK_H
