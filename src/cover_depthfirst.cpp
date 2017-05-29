#include <algorithm>
#include <argparse.h>
#include <deque>
#include <stdexcept>

#ifdef MULTITHREAD
#include <thread>
#endif

#include "global_settings.h"
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

void depthWorker(Context& context,
                 feature_size_t depth,
                 feature_size_t current);

void calcPriorities(Context& context,
                    feature_size_t depth,
                    bool& outUseAll,
                    feature_size_t* outPriorities,
                    feature_size_t& outPrioritiesLen);

void propagate(Context& context,
               feature_size_t depth,
               feature_size_t* columns,
               feature_size_t columnsLen);

bool checkAndAppend(Context& context,
                    feature_size_t depth);

void initArgParser(parser_t* parser)
{
}

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  set_size_t limit,
                  feature_size_t needCover) {

    test_feature_t nuim[uimSetLen * featuresLen];
    for (auto i=0; i<uimSetLen; ++i) {
        for (auto j=0; j<featuresLen; ++j) {
            nuim[i*featuresLen+j] = !!uim[i*featuresLen+j];
        }
    }

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
        calcPriorities(context, depth, useAll, priorities, prioritiesLen);
        if (useAll) {
            propagate(context, depth, priorities, prioritiesLen);
            depth += 1;
        } else {
            break;
        }
    }

    if (!checkAndAppend(context, depth)) {
        auto stepFeatureLen = context.getFeaturesLen(depth);

#ifdef MULTITHREAD
        std::vector<std::thread> threads(featuresLen);

        for (auto threadId = 0; threadId < stepFeatureLen; ++threadId) {
            START_COLLECT_TIME(threading, Counters::Threading);
            threads[threadId] = std::thread([threadId, &context, depth, &priorities]()
                                            {
                                                TimeCollector::ThreadInitialize();
                                                auto thContext = context.clone(depth);
                                                depthWorker(thContext, depth, priorities[threadId]);
                                                TimeCollector::ThreadFinalize();
                                            });
            STOP_COLLECT_TIME(threading);
        }

        for (auto threadId = 0; threadId < stepFeatureLen; ++threadId) {
            threads[threadId].join();
        }
#else
        for (auto i=0; i<stepFeatureLen; ++i) {
            depthWorker(context, depth, priorities[i]);
        }
#endif
    }
}

void calcPriorities(Context& context,
                    feature_size_t depth,
                    bool& outUseAll,
                    feature_size_t* outPriorities,
                    feature_size_t& outPrioritiesLen) {

    auto needCover = context.getNeedCover();
    auto uim = context.getUimSet(depth);
    auto uimSetLen = context.getUimSetLen(depth);
    auto featuresLen = context.getFeaturesLen(depth);
    auto currentCover = context.getCurrentCover(depth);
    auto currentColumns = context.getCurrentColumns(depth);

    DEBUG_BLOCK
        (
         getDebugStream() << "Calc priorities, input " << std::endl;

         for (auto j=0; j<featuresLen; ++j) {
             getDebugStream() << std::setw(3) << currentColumns[j];
         }
         getDebugStream() << std::endl;

         for (auto i=0; i<uimSetLen; ++i) {
             for (auto j=0; j<featuresLen; ++j) {
                 getDebugStream() << std::setw(3) << (uim[i* featuresLen + j] ? 1 : 0);
             }
             getDebugStream() << "  " << currentCover[i] << std::endl;
         }
         getDebugStream() << std::endl;
         );

    // Precalculate some common variables
    bool useMarkedColumns = false;
    bool markedColumns[featuresLen];
    set_size_t weights[featuresLen];
    std::fill(&markedColumns[0], &markedColumns[featuresLen], false);
    std::fill(&weights[0], &weights[featuresLen], false);

    for (auto i=0; i<uimSetLen; ++i) {
        feature_size_t count = 0;
        for (auto j=0; j<featuresLen; ++j) {
            count += uim[i * featuresLen + j];
            weights[j] += uim[i * featuresLen + j];
        }

        count += currentCover[i];

        if (count < needCover) {
            throw std::runtime_error("The covering cannot be found for the input data");
        } else if (count == needCover) {
            useMarkedColumns = true;
            for (auto j=0; j<featuresLen; ++j) {
                if (uim[i * featuresLen + j]) {
                    markedColumns[j] = true;
                }
            }
        }
    }

    outUseAll = useMarkedColumns;
    if (useMarkedColumns) {
        outPrioritiesLen = 0;
        for (auto j=0; j<featuresLen; ++j) {
            if (markedColumns[j]) {
                outPriorities[outPrioritiesLen++] = j;
            }
        }
    } else {
        std::tuple<set_size_t, feature_size_t> weights_sorted[featuresLen];
        for (auto j=0; j<featuresLen; ++j) {
            weights_sorted[j] = std::make_tuple(weights[j], j);
        }
        std::make_heap(&weights_sorted[0], &weights_sorted[featuresLen]);

        outPrioritiesLen = 0;
        for (auto j=0; j<featuresLen; ++j) {
            if (std::get<0>(weights_sorted[j]) == 0) {
                break;
            }
            outPriorities[outPrioritiesLen++] = std::get<1>(weights_sorted[j]);
        }
    }

    DEBUG_BLOCK
        (
         getDebugStream() << "Calc priorities, useAll: " << outUseAll << ", c: ";
         for (auto j=0; j<outPrioritiesLen; ++j) {
             getDebugStream() << currentColumns[outPriorities[j]] << " ";
         }
         getDebugStream() << std::endl << std::endl;
         );
}

void propagate(Context& context,
               feature_size_t depth,
               feature_size_t* columns,
               feature_size_t columnsLen) {

    auto needCover = context.getNeedCover();
    auto fullFeatureLen = context.getFeaturesLen(0);

    auto uim = context.getUimSet(depth);
    auto& uimSetLen = context.getUimSetLen(depth);
    auto& featuresLen = context.getFeaturesLen(depth);
    auto currentColumns = context.getCurrentColumns(depth);
    auto currentCover = context.getCurrentCover(depth);

    auto newUim = context.getUimSet(depth+1);
    auto& newUimSetLen = context.getUimSetLen(depth+1);
    auto& newFeaturesLen = context.getFeaturesLen(depth+1);
    auto newCurrentColumns = context.getCurrentColumns(depth+1);
    auto newCurrentCover = context.getCurrentCover(depth+1);

    DEBUG_BLOCK
        (
         getDebugStream() << "propagagate";
         for (auto j=0; j<columnsLen; ++j) {
             getDebugStream() << ' ' << currentColumns[columns[j]];
         }
         getDebugStream() << ", depth: " << depth << std::endl;

         for (auto j=0; j<featuresLen; ++j) {
             getDebugStream() << std::setw(3) << currentColumns[j];
         }
         getDebugStream() << std::endl;

         for (auto i=0; i<uimSetLen; ++i) {
             for (auto j=0; j<featuresLen; ++j) {
                 getDebugStream() << std::setw(3) << (uim[i* featuresLen + j] ? 1 : 0);
             }
             getDebugStream() << "  " << currentCover[i] << std::endl;
         }

         for (auto j=0; j<featuresLen; ++j) {
             getDebugStream() << std::setw(3) << context.getColor(depth, j);
         }
         getDebugStream() << std::endl;
         );

    newFeaturesLen = featuresLen - columnsLen;

    feature_size_t cs = 0;
    feature_size_t cj = 0;
    for (auto j=0; j<featuresLen; ++j) {
        if (cs < columnsLen && columns[cs] == j) {
            cs += 1;
            continue;
        }

        newCurrentColumns[cj++] = currentColumns[j];
    }

    newUimSetLen = 0;
    for (auto i=0; i<uimSetLen; ++i) {
        feature_size_t count = 0;
        for (auto j = 0; j < columnsLen; ++j) {
            count += uim[i * featuresLen + columns[j]];
        }
        count += currentCover[i];

        if (count < needCover) {
            newCurrentCover[newUimSetLen] = count;

            feature_size_t cs = 0;
            feature_size_t cj = 0;
            for (auto j=0; j<featuresLen; ++j) {
                if (cs < columnsLen && columns[cs] == j) {
                    cs += 1;
                    continue;
                }

                newUim[newFeaturesLen * newUimSetLen + (cj++)] = uim[i * featuresLen + j];
            }

            newUimSetLen += 1;
        }
    }

    for (auto j = 0; j < columnsLen; ++j) {
        context.setColor(depth, columns[j], depth+1);
    }

    DEBUG_BLOCK
        (
         getDebugStream() << "propagate result" << std::endl;

         for (auto j=0; j<newFeaturesLen; ++j) {
             getDebugStream() << std::setw(3) << newCurrentColumns[j];
         }
         getDebugStream() << std::endl;

         for (auto i=0; i<newUimSetLen; ++i) {
             for (auto j=0; j<newFeaturesLen; ++j) {
                 getDebugStream() << std::setw(3) << (newUim[i* newFeaturesLen + j] ? 1 : 0);
             }
             getDebugStream() << "  " << newCurrentCover[i] << std::endl;
         }

         for (auto j=0; j<newFeaturesLen; ++j) {
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
        calcPriorities(context, depth, useAll, priorities, prioritiesLen);
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
