#include <argparse.h>

#ifdef MULTITHREAD
#include <thread>
#endif

#include "global_settings.h"
#include "cover_generator.hpp"
#include "resultset.hpp"
#include "timecollector.hpp"

const set_size_t DEFAULT_WORK_BLOCK = 1024;

class Context final {

public:
    Context(feature_t* uimSet,
            set_size_t uimSetLen,
            feature_size_t featuresLen,
            ResultSet& resultSet,
            set_size_t limit,
            feature_size_t cover,
            CoverGenerator& generator,
            set_size_t workBlock) :
        _uimSet(uimSet),
        _uimSetLen(uimSetLen),
        _featuresLen(featuresLen),
        _resultSet(resultSet),
        _limit(limit),
        _cover(cover),
        _generator(generator),
        _workBlock(workBlock) { }

    inline feature_t* getUimSet() const { return _uimSet; }
    inline set_size_t getUimSetLen() const { return _uimSetLen; }
    inline feature_size_t getFeaturesLen() const { return _featuresLen; }
    inline ResultSet& getResultSet() const { return _resultSet; }
    inline set_size_t getLimit() const { return _limit; }
    inline feature_size_t getCover() const { return _cover; }
    inline CoverGenerator& getGenerator() const { return _generator; }
    inline set_size_t getWorkBlock() const { return _workBlock; }

#ifdef MULTITHREAD
    inline std::mutex& getGeneratorLock() { return _generatorLock; }
    inline std::mutex& getResultsLock() { return _resultsLock; }
#endif

private:
    feature_t* _uimSet;
    set_size_t _uimSetLen;
    feature_size_t _featuresLen;
    ResultSet& _resultSet;
    set_size_t _limit;
    feature_size_t _cover;
    CoverGenerator& _generator;
    set_size_t _workBlock;
#ifdef MULTITHREAD
    std::mutex _generatorLock;
    std::mutex _resultsLock;
#endif
};

parser_int_arg_t* work_block_arg;

void breadthWorker(Context& context);

void initArgParser(parser_t* parser)
{
    parser_int_add_arg(parser, &work_block_arg, "--work-block");
    parser_int_set_alt(work_block_arg, "-b");
    parser_int_set_help(work_block_arg, "amount of rows in working block");
    parser_int_set_default(work_block_arg, DEFAULT_WORK_BLOCK);
}

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  set_size_t limit,
                  feature_size_t needCover) {
    CoverGenerator generator(featuresLen);

    Context context(uim,
                    uimSetLen,
                    featuresLen,
                    resultSet,
                    limit,
                    needCover,
                    generator,
                    parser_int_get_value(work_block_arg));

#ifdef MULTITHREAD
    auto maxThreads = std::thread::hardware_concurrency();;
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
                coverKoef = 0;

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

