#include <algorithm>
#include <argparse.h>
#include <stdexcept>

#ifdef MULTITHREAD
#include <queue>
#include <mutex>
#include <thread>
#endif

#include "global_settings.h"
#include "cover_common.hpp"
#include "resultset.hpp"
#include "timecollector.hpp"

class Context final {

public:
    Context(test_feature_t* uimSet,
            feature_size_t* colors,
            set_size_t uimSetLen,
            feature_size_t featuresLen,
            ResultSet* resultSet,
#ifdef MULTITHREAD
            std::mutex* resultsLock,
#endif // MULTITHREAD
            set_size_t limit,
            feature_size_t needCover) :
        _colors(colors),
        _resultSet(resultSet),
#ifdef MULTITHREAD
        _resultsLock(resultsLock),
#endif // MULTITHREAD
        _limit(limit),
        _needCover(needCover) {

        _uimSets = new test_feature_t*[featuresLen+1];
        _uimSets[0] = uimSet;
        for (auto i = 1; i <= featuresLen; ++i) {
            _uimSets[i] = new test_feature_t[featuresLen * uimSetLen];
        }

        _uimSetLens = new set_size_t[featuresLen+1];
        _uimSetLens[0] = uimSetLen;

        _featuresLens = new feature_size_t[featuresLen+1];
        _featuresLens[0] = featuresLen;

        _currentColumns = new feature_size_t*[featuresLen+1];
        _currentCovers = new feature_size_t*[featuresLen+1];

        for (auto i = 0; i <= featuresLen; ++i) {
            _currentColumns[i] = new feature_size_t[featuresLen];
            _currentCovers[i] = new feature_size_t[uimSetLen];
        }

        for (auto j = 0; j < featuresLen; ++j) {
            _currentColumns[0][j] = j;
        }
        std::fill(&_currentCovers[0][0], &_currentCovers[0][uimSetLen], 0);
    }

    ~Context() {
        if (_uimSets == nullptr) {
            return;
        }

        for (auto i = 1; i <= _featuresLens[0]; ++i) {
            delete [] _uimSets[i];
        }

        for (auto i = 0; i <= _featuresLens[0]; ++i) {
            delete [] _currentColumns[i];
            delete [] _currentCovers[i];
        }

        delete [] _uimSets;
        delete [] _uimSetLens;
        delete [] _featuresLens;
        delete [] _currentColumns;
        delete [] _currentCovers;
    }

    Context(Context&& another) :
        _uimSets(another._uimSets),
        _colors(another._colors),
        _uimSetLens(another._uimSetLens),
        _featuresLens(another._featuresLens),
        _currentColumns(another._currentColumns),
        _currentCovers(another._currentCovers),
        _resultSet(another._resultSet),
#ifdef MULTITHREAD
        _resultsLock(another._resultsLock),
#endif // MULTITHREAD
        _limit(another._limit),
        _needCover(another._needCover)
        {
        another._uimSets = nullptr;
        another._uimSetLens = nullptr;
        another._featuresLens = nullptr;
        another._currentColumns = nullptr;
        another._currentCovers = nullptr;
    }

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Context clone(feature_size_t depth) {
        auto temp = Context(_uimSets[0],
                            _colors,
                            _uimSetLens[0],
                            _featuresLens[0],
                            _resultSet,
#ifdef MULTITHREAD
                            _resultsLock,
#endif // MULTITHREAD
                            _limit,
                            _needCover);

        for (auto i = 1; i <= depth; ++i) {
            for (auto j = 0; j < _uimSetLens[i] * _featuresLens[i]; ++j) {
                temp._uimSets[i][j] = _uimSets[i][j];
            }
            temp._uimSetLens[i] = _uimSetLens[i];
            temp._featuresLens[i] = _featuresLens[i];
            for (auto j = 0; j < _featuresLens[i]; ++j) {
                temp._currentColumns[i][j] = _currentColumns[i][j];
                temp._currentCovers[i][j] = _currentCovers[i][j];
            }
        }

        return temp;
    }

    inline test_feature_t* getUimSet(feature_size_t depth) { return _uimSets[depth]; }
    inline set_size_t& getUimSetLen(feature_size_t depth) { return _uimSetLens[depth]; }
    inline feature_size_t& getFeaturesLen(feature_size_t depth) { return _featuresLens[depth]; }
    inline feature_size_t* getCurrentColumns(feature_size_t depth) { return _currentColumns[depth]; }
    inline feature_size_t* getCurrentCover(feature_size_t depth) { return _currentCovers[depth]; }

    inline feature_size_t getColor(feature_size_t depth, feature_size_t id) {
        return _colors[_currentColumns[depth][id]];
    }
    inline void setColor(feature_size_t depth, feature_size_t id, feature_size_t value) {
        _colors[_currentColumns[depth][id]] = value;
    }

    inline ResultSet& getResultSet() const { return *_resultSet; }
    inline set_size_t getLimit() const { return _limit; }
    inline feature_size_t getNeedCover() const { return _needCover; }

#ifdef MULTITHREAD
    inline std::mutex& getResultsLock() const { return *_resultsLock; }
#endif

private:
    test_feature_t** _uimSets;
    feature_size_t* _colors;
    set_size_t* _uimSetLens;
    feature_size_t* _featuresLens;
    feature_size_t** _currentColumns;
    feature_size_t** _currentCovers;

    ResultSet* _resultSet;
    set_size_t _limit;
    feature_size_t _needCover;
#ifdef MULTITHREAD
    std::mutex* _resultsLock;
#endif
};

parser_int_arg_t* thread_count_arg;

void depthWorker(Context& context,
                 feature_size_t depth,
                 feature_size_t current);

void propagate(Context& context,
               feature_size_t depth,
               feature_size_t* columns,
               feature_size_t columnsLen);

bool checkAndAppend(Context& context,
                    feature_size_t depth);

void initArgParser(parser_t* parser)
{
    parser_int_add_arg(parser, &thread_count_arg, "--thread-count");
    parser_int_set_alt(thread_count_arg, "-t");
    parser_int_set_help(thread_count_arg, "use forced number of threads");
    parser_int_set_default(thread_count_arg, 0);
}

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  set_size_t limit,
                  feature_size_t needCover) {

    test_feature_t nuim[uimSetLen * featuresLen];
    CoverCommon::binarize(uim, uimSetLen, featuresLen, nuim);

    feature_size_t colors[featuresLen];
    std::fill(&colors[0], &colors[featuresLen], featuresLen+1);

    IF_MULTITHREAD(std::mutex resultsLock);

    Context context(nuim,
                    colors,
                    uimSetLen,
                    featuresLen,
                    &resultSet,
#ifdef MULTITHREAD
                    &resultsLock,
#endif
                    limit,
                    needCover);

    feature_size_t depth = 0;
    bool useAll;
    feature_size_t priorities[featuresLen];
    feature_size_t prioritiesLen;

    for (;;) {
        CoverCommon::calcPriorities(context.getUimSet(depth),
                                    context.getUimSetLen(depth),
                                    context.getFeaturesLen(depth),
                                    context.getNeedCover(),
                                    context.getCurrentCover(depth),
                                    context.getCurrentColumns(depth),
                                    useAll,
                                    priorities,
                                    prioritiesLen);

        if (useAll) {
            propagate(context, depth, priorities, prioritiesLen);
            depth += 1;
        } else {
            break;
        }
    }

    if (!checkAndAppend(context, depth)) {
#ifdef MULTITHREAD
        std::queue<feature_size_t> tasks;
        for (auto i = 0; i<prioritiesLen; ++i) {
            tasks.push(i);
        }

        auto maxThreads = parser_int_get_value(thread_count_arg);
        if (maxThreads == 0) {
            maxThreads = std::thread::hardware_concurrency();
        }
        if (maxThreads > prioritiesLen) {
            maxThreads = prioritiesLen;
        }

        std::mutex mutex;
        std::vector<std::thread> threads(maxThreads);
        for (auto threadId = 0; threadId < maxThreads; ++threadId) {
            START_COLLECT_TIME(threading, Counters::Threading);
            threads[threadId] = std::thread([threadId, &context, depth, &priorities, &tasks, &mutex]()
                                            {
                                                TimeCollector::ThreadInitialize();
                                                auto thContext = context.clone(depth);
                                                for(;;) {
                                                    feature_size_t id = 0;
                                                    {
                                                        std::unique_lock<std::mutex> mlock(mutex);
                                                        if (tasks.empty()) {
                                                            break;
                                                        }
                                                        id = tasks.front();
                                                        tasks.pop();
                                                    }
                                                    depthWorker(thContext, depth, priorities[id]);
                                                }
                                                TimeCollector::ThreadFinalize();
                                            });
            STOP_COLLECT_TIME(threading);
        }

        for (auto threadId = 0; threadId < maxThreads; ++threadId) {
            threads[threadId].join();
        }
#else
        for (auto i=0; i<prioritiesLen; ++i) {
            depthWorker(context, depth, priorities[i]);
        }
#endif
    }
}

void propagate(Context& context,
               feature_size_t depth,
               feature_size_t* columns,
               feature_size_t columnsLen) {

    DEBUG_BLOCK
        (
         getDebugStream() << "propagagate, input";
         for (auto j=0; j<columnsLen; ++j) {
             getDebugStream() << ' ' << context.getCurrentColumns(depth)[columns[j]];
         }
         getDebugStream() << ", depth: " << depth << std::endl;
         );

    CoverCommon::reduceUim(context.getUimSet(depth),
                           context.getUimSetLen(depth),
                           context.getFeaturesLen(depth),
                           context.getCurrentColumns(depth),
                           context.getCurrentCover(depth),
                           context.getUimSet(depth+1),
                           context.getUimSetLen(depth+1),
                           context.getFeaturesLen(depth+1),
                           context.getCurrentColumns(depth+1),
                           context.getCurrentCover(depth+1),
                           context.getNeedCover(),
                           columns,
                           columnsLen);

    for (auto j = 0; j < columnsLen; ++j) {
        context.setColor(depth, columns[j], depth+1);
    }

    DEBUG_BLOCK
        (
         getDebugStream() << "propagate, output" << std::endl;
         for (auto j=0; j<context.getFeaturesLen(depth+1); ++j) {
             getDebugStream() << std::setw(3) << context.getColor(depth+1, j);
         }
         getDebugStream() << std::endl;
         );
}

bool checkAndAppend(Context& context,
                    feature_size_t depth) {

    if (context.getUimSetLen(depth) == 0) {
        auto fullFeatureLen = context.getFeaturesLen(0);
        auto featuresLen = context.getFeaturesLen(depth);
        auto currentColumns = context.getCurrentColumns(depth);

        test_feature_t covering[fullFeatureLen];
        std::fill(&covering[0], &covering[fullFeatureLen], 1);
        for (auto j = 0; j < featuresLen; ++j) {
            covering[currentColumns[j]] = 0;
        }

        IF_MULTITHREAD(context.getResultsLock().lock());
        context.getResultSet().append(covering);
        IF_MULTITHREAD(context.getResultsLock().unlock());
        return true;
    }

    return false;
}

void depthWorker(Context& context,
                 feature_size_t depth,
                 feature_size_t current) {

    auto cost = context.getFeaturesLen(0) - context.getFeaturesLen(depth);
    if (cost >= context.getResultSet().getCostBarrier()) {
        return;
    }

    propagate(context, depth, &current, 1);
    depth += 1;

    if (checkAndAppend(context, depth)) {
        return;
    }

    bool useAll;
    feature_size_t priorities[context.getFeaturesLen(depth)];
    feature_size_t prioritiesLen;

    for (;;) {
        CoverCommon::calcPriorities(context.getUimSet(depth),
                                    context.getUimSetLen(depth),
                                    context.getFeaturesLen(depth),
                                    context.getNeedCover(),
                                    context.getCurrentCover(depth),
                                    context.getCurrentColumns(depth),
                                    useAll,
                                    priorities,
                                    prioritiesLen);

        if (useAll) {
            propagate(context, depth, priorities, prioritiesLen);
            depth += 1;
        } else {
            break;
        }
    }

    for (auto i=0; i<prioritiesLen; ++i) {
        auto handle = depth <= context.getColor(depth, priorities[i]);

        DEBUG_INFO("column " << context.getCurrentColumns(depth)[priorities[i]]
                   << ", handle: " << handle
                   << ", depth: " << depth
                   << ", prevColor: " << context.getColor(depth, priorities[i]));

        if (handle) {
            depthWorker(context, depth, priorities[i]);
        }
    }
}
