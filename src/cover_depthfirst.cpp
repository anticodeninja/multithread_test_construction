#include <algorithm>
#include <deque>
#include <argparse.h>

#ifdef MULTITHREAD
#include <thread>
#endif

#include "global_settings.h"
#include "resultset.hpp"
#include "timecollector.hpp"

class Context final {

public:
    Context(test_feature_t* uimSet,
            set_size_t uimSetLen,
            feature_size_t featuresLen,
            ResultSet& resultSet,
            set_size_t limit,
            feature_size_t cover) :
        _uimSet(uimSet),
        _uimSetLen(uimSetLen),
        _featuresLen(featuresLen),
        _resultSet(resultSet),
        _limit(limit),
        _cover(cover),
        _costBarrier(std::numeric_limits<set_size_t>::max()) { }

    inline test_feature_t* getUimSet() const { return _uimSet; }
    inline set_size_t getUimSetLen() const { return _uimSetLen; }
    inline feature_size_t getFeaturesLen() const { return _featuresLen; }
    inline ResultSet& getResultSet() const { return _resultSet; }
    inline set_size_t getLimit() const { return _limit; }
    inline feature_size_t getCover() const { return _cover; }
    inline set_size_t getCostBarrier() const { return _costBarrier; }
    inline void setCostBarrier(set_size_t value) { _costBarrier = value; }

#ifdef MULTITHREAD
    inline std::mutex& getResultsLock() { return _resultsLock; }
#endif

private:
    test_feature_t* _uimSet;
    set_size_t _uimSetLen;
    feature_size_t _featuresLen;
    ResultSet& _resultSet;
    set_size_t _limit;
    feature_size_t _cover;
    set_size_t _costBarrier;
#ifdef MULTITHREAD
    std::mutex _resultsLock;
#endif
};

class CoverDepthTask final {
 public:
    CoverDepthTask(feature_size_t column,
                   feature_size_t featuresCount,
                   test_feature_t* mask,
                   set_size_t rowsCount,
                   test_feature_t** rows,
                   feature_size_t* covering) {
        _column = column;

        _mask = new test_feature_t[featuresCount];
        for (auto i=0; i<featuresCount; ++i) {
            _mask[i] = mask[i];
        }

        _rowsCount = rowsCount;
        _rows = new test_feature_t*[rowsCount];
        _covering = new feature_size_t[rowsCount];
        for (auto i=0; i<rowsCount; ++i) {
            _rows[i] = rows[i];
            _covering[i] = covering[i];
        }
    }

    static CoverDepthTask* createInitTask(feature_size_t column,
                                          test_feature_t* uim,
                                          set_size_t rowsCount,
                                          feature_size_t featuresCount) {
        test_feature_t mask[featuresCount];
        for (auto i=0; i<featuresCount; ++i) {
            mask[i] = 0;
        }

        test_feature_t* rows[rowsCount];
        feature_size_t covering[rowsCount];
        for (auto i=0; i<rowsCount; ++i) {
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

    inline feature_size_t getColumn() const { return _column; }
    inline test_feature_t* getMask() const { return _mask; }

    inline set_size_t getRowsCount() const { return _rowsCount; }
    inline test_feature_t** getRows() const { return _rows; }
    inline feature_size_t* getCovering() const { return _covering; }

private:
    feature_size_t _column;
    test_feature_t* _mask;

    set_size_t _rowsCount;
    test_feature_t** _rows;
    feature_size_t* _covering;
};

void depthWorker(Context& context, std::deque<CoverDepthTask*>& taskQueue);

void initArgParser(parser_t* parser)
{
}

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  int limit,
                  int cover) {
    auto nuim = new test_feature_t[uimSetLen * featuresLen];
    for (auto i=0; i<uimSetLen; ++i) {
        for (auto j=0; j<featuresLen; ++j) {
            nuim[i*featuresLen+j] = !!uim[i*featuresLen+j];
        }
    }

    set_size_t weights[featuresLen];
    std::tuple<set_size_t, feature_size_t> weights_sorted[featuresLen];

    for (auto i=0; i<featuresLen; ++i) {
        weights[i] = 0;
    }
    for (auto i=0; i<uimSetLen; ++i) {
        for (auto j=0; j<featuresLen; ++j) {
            weights[j] += uim[i * featuresLen + j];
        }
    }

    for (auto i=0; i<featuresLen; ++i) {
        weights_sorted[i] = std::make_tuple(weights[i], i);
    }

    Context context(nuim,
                    uimSetLen,
                    featuresLen,
                    resultSet,
                    limit,
                    cover);

    std::sort(&weights_sorted[0], &weights_sorted[featuresLen]);

#ifdef MULTITHREAD
    std::vector<std::thread> threads(featuresLen);
    std::vector<std::deque<CoverDepthTask*>> tasks(featuresLen);

    for (auto threadId = 0; threadId < featuresLen; ++threadId) {
        tasks[threadId].push_front(CoverDepthTask::createInitTask(threadId,
                                                                  nuim,
                                                                  uimSetLen,
                                                                  featuresLen));
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([threadId, &context, &tasks]()
        {
            TimeCollector::ThreadInitialize();
            depthWorker(context, tasks[threadId]);
            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }

    for (auto threadId = 0; threadId < context.getFeaturesLen(); ++threadId) {
        threads[threadId].join();
    }
#else
    std::deque<CoverDepthTask*> tasks;
    for (auto i=0; i<featuresLen; ++i) {
        tasks.push_front(CoverDepthTask::createInitTask(i,
                                                        nuim,
                                                        uimSetLen,
                                                        featuresLen));
    }
    depthWorker(context, tasks);
#endif
}

void depthWorker(Context& context, std::deque<CoverDepthTask*>& tasks) {
    test_feature_t index = 0;
    test_feature_t* rows[context.getUimSetLen()];
    feature_size_t covering[context.getUimSetLen()];
    test_feature_t mask[context.getFeaturesLen()];
    set_size_t weights[context.getFeaturesLen()];
    std::tuple<set_size_t, feature_size_t> weights_sorted[context.getFeaturesLen()];
    CoverDepthTask* task;

    for(;;) {
        if (tasks.size() == 0) {
            return;
        }

        task = tasks.front();
        tasks.pop_front();

        Result result = Result(task->getMask(), context.getFeaturesLen());

        if (result.getCost() >= context.getCostBarrier()) {
            delete task;
            continue;
        }

        if (task->getRowsCount() == 0) {
            IF_MULTITHREAD(context.getResultsLock().lock());
            if (context.getResultSet().append(std::move(result)) != ResultSet::IGNORED) {
                if (context.getResultSet().isFull()) {
                    context.setCostBarrier(context.getResultSet().get(context.getResultSet().getSize() - 1).getCost());
                }
            }
            IF_MULTITHREAD(context.getResultsLock().unlock());
            delete task;
            continue;
        }

        for (auto i=0; i<context.getFeaturesLen(); ++i) {
            mask[i] = task->getMask()[i];
        }
        mask[task->getColumn()] = 1;

        index = 0;
        for (auto i=0; i<task->getRowsCount(); ++i) {
            auto coverKoef = task->getCovering()[i] + task->getRows()[i][task->getColumn()];
            if (coverKoef < context.getCover()) {
                rows[index] = task->getRows()[i];
                covering[index] = coverKoef;
                index += 1;
            }
        }

        for (auto i=0; i<context.getFeaturesLen(); ++i) {
            weights[i] = 0;
        }
        for (auto i=0; i<index; ++i) {
            for (auto j=0; j<context.getFeaturesLen(); ++j) {
                weights[j] += rows[i][j];
            }
        }

        for (auto i=0; i<context.getFeaturesLen(); ++i) {
            weights_sorted[i] = std::make_tuple(weights[i], i);
        }

        std::sort(&weights_sorted[0], &weights_sorted[context.getFeaturesLen()]);

        for(auto i=0; i<context.getFeaturesLen(); ++i) {
            if (!mask[i]) {
                tasks.push_front(new CoverDepthTask(i, context.getFeaturesLen(), mask, index, rows, covering));
            }
        }

        delete task;
    }
}
