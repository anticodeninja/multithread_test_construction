#include <argparse.h>

#include <algorithm>

#ifdef MULTITHREAD
#include <thread>
#endif

#include "global_settings.h"
#include "cover_common.hpp"
#include "cover_generator.hpp"
#include "resultset.hpp"
#include "timecollector.hpp"

const set_size_t DEFAULT_WORK_BLOCK = 1024;

class Context final {

public:
    Context(test_feature_t* uimSet,
            set_size_t uimSetLen,
            feature_size_t featuresLen,
            ResultSet& resultSet,
            set_size_t limit,
            feature_size_t cover,
            feature_size_t* currentCover,
            CoverGenerator& generator,
            set_size_t workBlock) :
        _uimSet(uimSet),
        _uimSetLen(uimSetLen),
        _featuresLen(featuresLen),
        _resultSet(resultSet),
        _limit(limit),
        _cover(cover),
        _currentCover(currentCover),
        _generator(generator),
        _workBlock(workBlock) { }

    inline test_feature_t* getUimSet() const { return _uimSet; }
    inline set_size_t getUimSetLen() const { return _uimSetLen; }
    inline feature_size_t getFeaturesLen() const { return _featuresLen; }
    inline ResultSet& getResultSet() const { return _resultSet; }
    inline set_size_t getLimit() const { return _limit; }
    inline feature_size_t getCover() const { return _cover; }
    inline feature_size_t* getCurrentCover() const { return _currentCover; }
    inline CoverGenerator& getGenerator() const { return _generator; }
    inline set_size_t getWorkBlock() const { return _workBlock; }

#ifdef MULTITHREAD
    inline std::mutex& getGeneratorLock() { return _generatorLock; }
    inline std::mutex& getResultsLock() { return _resultsLock; }
#endif

private:
    test_feature_t* _uimSet;
    set_size_t _uimSetLen;
    feature_size_t _featuresLen;
    ResultSet& _resultSet;
    set_size_t _limit;
    feature_size_t _cover;
    feature_size_t* _currentCover;
    CoverGenerator& _generator;
    set_size_t _workBlock;
#ifdef MULTITHREAD
    std::mutex _generatorLock;
    std::mutex _resultsLock;
#endif
};

parser_int_arg_t* work_block_arg;

parser_int_arg_t* thread_count_arg;

void breadthWorker(Context& context);

void initArgParser(parser_t* parser)
{
    parser_int_add_arg(parser, &work_block_arg, "--work-block");
    parser_int_set_alt(work_block_arg, "-b");
    parser_int_set_help(work_block_arg, "amount of rows in working block");
    parser_int_set_default(work_block_arg, DEFAULT_WORK_BLOCK);

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

    feature_size_t currentCover[uimSetLen];
    std::fill(&currentCover[0], &currentCover[uimSetLen], 0);

#ifdef PREPARE_DATA
    auto origFeaturesLen = featuresLen;
    auto& origResultSet = resultSet;

    test_feature_t marked[featuresLen];
    std::fill(&marked[0], &marked[featuresLen], 0);

    feature_size_t currentColumns[featuresLen];
    for (auto j = 0; j < featuresLen; ++j) {
        currentColumns[j] = j;
    }

    bool useAll;
    feature_size_t priorities[featuresLen];
    feature_size_t prioritiesLen;

    for (;;) {
        CoverCommon::calcPriorities(nuim, uimSetLen, featuresLen, needCover, currentCover, currentColumns,
                                    useAll, priorities, prioritiesLen);

        if (useAll) {
            for (auto j = 0; j < prioritiesLen; ++j) {
                marked[currentColumns[priorities[j]]] = 1;
            }

            CoverCommon::reduceUim(nuim, uimSetLen, featuresLen, currentColumns, currentCover,
                                   nuim, uimSetLen, featuresLen, currentColumns, currentCover,
                                   needCover, priorities, prioritiesLen);
        } else {
            std::reverse(&priorities[0], &priorities[prioritiesLen]);
            CoverCommon::reorderUim(nuim, uimSetLen, featuresLen, currentColumns,
                                    nuim, uimSetLen, featuresLen, currentColumns,
                                    priorities, prioritiesLen);
            break;
        }
    }

    ResultSet tempResultSet(resultSet.getLimit(), featuresLen);
#else
    ResultSet& tempResultSet = resultSet;
#endif

    CoverGenerator generator(featuresLen);
    Context context(nuim,
                    uimSetLen,
                    featuresLen,
                    tempResultSet,
                    limit,
                    needCover,
                    currentCover,
                    generator,
                    parser_int_get_value(work_block_arg));

#ifdef MULTITHREAD
    auto maxThreads = parser_int_get_value(thread_count_arg);
    if (maxThreads == 0) {
        maxThreads = std::thread::hardware_concurrency();
    }

    std::vector<std::thread> threads(maxThreads);
    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([threadId, &context]()
        {
            TimeCollector::ThreadInitialize();
            breadthWorker(context);
            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }

    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        threads[threadId].join();
    }
#else
    breadthWorker(context);
#endif

#ifdef PREPARE_DATA
    test_feature_t tempTestRow[origFeaturesLen];
    for(auto i=0; i<tempResultSet.getSize(); ++i) {
        std::fill(&tempTestRow[0], &tempTestRow[origFeaturesLen], 0);
        for (auto j = 0; j < origFeaturesLen; ++j) {
            if (marked[j]) {
                tempTestRow[j] = 1;
            }
        }
        for (auto j = 0; j < featuresLen; ++j) {
            if (tempResultSet.get(i)[j]) {
                tempTestRow[currentColumns[j]] = 1;
            }
        }
        resultSet.append(tempTestRow);
    }
#endif
}

void breadthWorker(Context& context) {
    uint_fast32_t _count;
    uint_fast8_t _tasks[context.getFeaturesLen() * context.getWorkBlock()];
    bool coverAll;
    bool workIsEnd;
    uint_fast16_t coverKoef;

    for(;;) {
        workIsEnd = false;

        IF_MULTITHREAD(context.getResultsLock().lock());
        if (context.getResultSet().isFull()) {
            workIsEnd = true;
        }
        IF_MULTITHREAD(context.getResultsLock().unlock());

        if (workIsEnd) {
            return;
        }

        IF_MULTITHREAD(context.getGeneratorLock().lock());
        _count = context.getGenerator().next(_tasks, context.getWorkBlock());
        IF_MULTITHREAD(context.getGeneratorLock().unlock());

        if (_count == 0) {
            return;
        }

        for(uint_fast32_t i=0; i<_count; ++i) {
            coverAll = true;

            for (uint_fast32_t r=0; r<context.getUimSetLen(); ++r) {
                coverKoef = context.getCurrentCover()[r];;

                for(uint_fast16_t j=0; j<context.getFeaturesLen(); ++j) {
                    if (_tasks[i * context.getFeaturesLen() + j] && context.getUimSet()[r * context.getFeaturesLen() + j]) {
                        coverKoef += 1;
                        if (coverKoef == context.getCover()) {
                            break;
                        }
                    }
                }

                if (coverKoef < context.getCover()) {
                    coverAll = false;
                    break;
                }
            }

            if (coverAll) {
                IF_MULTITHREAD(context.getResultsLock().lock());
                context.getResultSet().append(&_tasks[i * context.getFeaturesLen()]);
                IF_MULTITHREAD(context.getResultsLock().unlock());
            }
        }
    }
}
