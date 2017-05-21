#include <algorithm>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <chrono>
#include <deque>
#include <vector>
#include <thread>

#include "../argparse-port/argparse.h"

#include "timecollector.hpp"
#include "datafile.hpp"
#include "resultset.hpp"

#include "cover_generator.hpp"
#include "cover_depth_task.hpp"

#ifdef CUDA
#include "cudacover.h"
#endif

#if MULTITHREAD
#define IF_MULTITHREAD(command) command
#else
#define IF_MULTITHREAD(command);
#endif

DataFile dataFile;

uint_fast8_t cover;

uint_fast8_t* uim;
uint_fast32_t performedTasks;
ResultSet* results;
std::mutex* resultsLock;

uint_fast16_t costBarrier;

const uint_fast32_t TAKE_AMOUNT = 1024;
CoverGenerator* generator;
std::mutex* generatorLock;

#if defined(DEPTH_ALGO)

void findInDepth();
void depthWorker(std::deque<CoverDepthTask*>& taskQueue);

#elif defined(BREADTH_ALGO)

void findInBreadth();
void breadthWorker();

#else

#error Cover algo is not chosen

#endif


#ifdef COVER_PROGRAM

int main(int argc, char** argv)
{
    parser_t* parser;
    parser_init(&parser);

    parser_string_arg_t* input_arg;
    parser_string_add_arg(parser, &input_arg, "input");
    parser_string_set_help(input_arg, "input file");

    parser_string_arg_t* output_arg;
    parser_string_add_arg(parser, &output_arg, "output");
    parser_string_set_help(output_arg, "output file");

    parser_int_arg_t* covering_arg;
    parser_int_add_arg(parser, &covering_arg, "--covering");
    parser_int_set_alt(covering_arg, "-c");
    parser_int_set_help(covering_arg, "amount of column which should be covered");
    parser_int_set_default(covering_arg, 1);

    parser_int_arg_t* result_limit_arg;
    parser_int_add_arg(parser, &result_limit_arg, "--result-limit");
    parser_int_set_alt(result_limit_arg, "-l");
    parser_int_set_help(result_limit_arg, "maximal amount of result");
    parser_int_set_default(result_limit_arg, 10);

    parser_flag_arg_t* no_transfer;
    parser_flag_add_arg(parser, &no_transfer, "--no-transfer");
    parser_flag_set_help(no_transfer, "no transfer blocks from input file to output");

    if (parser_parse(parser, argc, argv) != PARSER_RESULT_OK) {
        printf("%s", parser_get_last_err(parser));
        parser_free(&parser);
        return 1;
    }

    TimeCollector::Initialize();
    TimeCollector::ThreadInitialize();
    TimeCollectorEntry executionTime(Counters::All);

    START_COLLECT_TIME(readingInput, Counters::ReadingInput);
    if (strcmp("-", parser_string_get_value(input_arg)) != 0) {
        std::ifstream input_stream(parser_string_get_value(input_arg));
        dataFile.load(input_stream);
    } else {
        dataFile.load(std::cin);
    }

    uim = new uint_fast8_t[dataFile.getUimSetLen() * dataFile.getFeaturesLen()];
    for(auto i=0; i<dataFile.getUimSetLen(); ++i) {
        for(auto j=0; j<dataFile.getFeaturesLen(); ++j) {
            uim[i * dataFile.getFeaturesLen() + j] = dataFile.getUimSet()[i * dataFile.getFeaturesLen() + j];
        }
    }
    STOP_COLLECT_TIME(readingInput);

    cover = parser_int_get_value(covering_arg);
    results = new ResultSet(parser_int_get_value(result_limit_arg));

    IF_MULTITHREAD(resultsLock = new std::mutex());

    performedTasks = 0;

#if defined(DEPTH_ALGO)
    findInDepth();
#elif defined(BREADTH_ALGO)
    findInBreadth();
#else
    #error Cover algo is not chosen
#endif

    auto featuresLen = dataFile.getFeaturesLen();
    if (parser_flag_is_filled(no_transfer)) {
        dataFile.reset();
    }

    auto tests = new feature_t[results->getSize() * featuresLen];
    for(auto i=0; i<results->getSize(); ++i) {
        for(auto j=0; j<featuresLen; ++j) {
            tests[i * featuresLen + j] = results->get(i).get(j);
        }
    }

    dataFile.setTestSetBlock(tests, cover, results->getSize(), featuresLen);

    if (strcmp("-", output_arg->value) != 0) {
        std::ofstream output_stream(output_arg->value);
        dataFile.save(output_stream);
    } else {
        dataFile.save(std::cout);
    }

    executionTime.Stop();

    delete uim;
    delete results;

    IF_MULTITHREAD(delete resultsLock);

    TimeCollector::ThreadFinalize();
    std::ofstream timeCollectorOutput("current_profile.txt");
    TimeCollector::PrintInfo(timeCollectorOutput);

    parser_free(&parser);
    return 0;
}

#endif

#ifdef DEPTH_ALGO
void findInDepth() {
    costBarrier = UINT_FAST16_MAX;

    uint_fast32_t weights[dataFile.getFeaturesLen()];
    std::tuple<uint_fast32_t, uint_fast16_t> weights_sorted[dataFile.getFeaturesLen()];

    for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
        weights[i] = 0;
    }
    for(uint_fast32_t i=0; i<dataFile.getUimSetLen(); ++i) {
        for(uint_fast16_t j=0; j<dataFile.getFeaturesLen(); ++j) {
            weights[j] += uim[i * dataFile.getFeaturesLen() + j];
        }
    }

    for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
        weights_sorted[i] = std::make_tuple(weights[i], i);
    }

    std::sort(&weights_sorted[0], &weights_sorted[dataFile.getFeaturesLen()]);

#ifdef MULTITHREAD
    std::vector<std::thread> threads(dataFile.getFeaturesLen());
    std::vector<std::deque<CoverDepthTask*>> tasks(dataFile.getFeaturesLen());

    for(uint_fast16_t threadId = 0; threadId < dataFile.getFeaturesLen(); ++threadId) {
        tasks[threadId].push_front(CoverDepthTask::createInitTask(threadId,
                                                                  uim,
                                                                  dataFile.getUimSetLen(),
                                                                  dataFile.getFeaturesLen()));
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([threadId, &tasks]()
        {
            TimeCollector::ThreadInitialize();
            depthWorker(tasks[threadId]);
            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }

    for(uint_fast16_t threadId = 0; threadId < dataFile.getFeaturesLen(); ++threadId) {
        threads[threadId].join();
    }
#else
    std::deque<CoverDepthTask*> tasks;
    for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
        tasks.push_front(CoverDepthTask::createInitTask(i,
                                                        uim,
                                                        dataFile.getUimSetLen(),
                                                        dataFile.getFeaturesLen()));
    }
    depthWorker(tasks);
#endif
}

void depthWorker(std::deque<CoverDepthTask*>& tasks) {
    uint_fast32_t index = 0;
    uint_fast8_t* rows[dataFile.getUimSetLen()];
    uint_fast8_t covering[dataFile.getUimSetLen()];
    uint_fast8_t mask[dataFile.getFeaturesLen()];
    uint_fast32_t weights[dataFile.getFeaturesLen()];
    std::tuple<uint_fast32_t, uint_fast16_t> weights_sorted[dataFile.getFeaturesLen()];
    CoverDepthTask* task;

    for(;;) {
        if (tasks.size() == 0) {
            return;
        }

        task = tasks.front();
        tasks.pop_front();

        Result result = Result(task->getMask(), dataFile.getFeaturesLen());

        if (result.getCost() >= costBarrier) {
            delete task;
            continue;
        }

        performedTasks += 1;

        if (task->getRowsCount() == 0) {
            IF_MULTITHREAD(resultsLock->lock());
            if (results->append(std::move(result)) != ResultSet::IGNORED) {
                if (results->isFull()) {
                    costBarrier = results->get(results->getSize() - 1).getCost();
                }
            }
            IF_MULTITHREAD(resultsLock->unlock());
            delete task;
            continue;
        }

        for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
            mask[i] = task->getMask()[i];
        }
        mask[task->getColumn()] = 1;

        index = 0;
        for(uint_fast32_t i=0; i<task->getRowsCount(); ++i) {
            uint_fast16_t coverKoef = task->getCovering()[i] + task->getRows()[i][task->getColumn()];
            if (coverKoef < cover) {
                rows[index] = task->getRows()[i];
                covering[index] = coverKoef;
                index += 1;
            }
        }

        for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
            weights[i] = 0;
        }
        for(uint_fast32_t i=0; i<index; ++i) {
            for(uint_fast16_t j=0; j<dataFile.getFeaturesLen(); ++j) {
                weights[j] += rows[i][j];
            }
        }

        for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
            weights_sorted[i] = std::make_tuple(weights[i], i);
        }

        std::sort(&weights_sorted[0], &weights_sorted[dataFile.getFeaturesLen()]);

        for(uint_fast16_t i=0; i<dataFile.getFeaturesLen(); ++i) {
            if (!mask[i]) {
                tasks.push_front(new CoverDepthTask(i, dataFile.getFeaturesLen(), mask, index, rows, covering));
            }
        }

        delete task;
    }
}
#endif

#ifdef BREADTH_ALGO

void findInBreadth() {
    generator = new CoverGenerator(dataFile.getFeaturesLen());

#ifdef MULTITHREAD
    generatorLock = new std::mutex();

    auto maxThreads = std::thread::hardware_concurrency();;
    std::vector<std::thread> threads(maxThreads);

    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([threadId]()
        {
            TimeCollector::ThreadInitialize();
            breadthWorker();
            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }

    for(auto threadId = 0; threadId < maxThreads; ++threadId) {
        threads[threadId].join();
    }

    delete generatorLock;
#else
    breadthWorker();
#endif

}

#ifdef CUDA

void breadthWorker() {
    uint_fast32_t _count;

    cudacover_t* ctx;
    cudacover_init(&ctx, rowsCount, featuresCount, TAKE_AMOUNT);
    for(uint_fast32_t i=0; i<rowsCount; ++i) {
        for(uint_fast16_t j=0; j<featuresCount; ++j) {
            ctx->lset[i * featuresCount + j] = uim[i * featuresCount + j];
        }
    }
    ctx->cover_koef = cover;

    for(;;) {
        if (results->isFull()) {
            return;
        }

        if ((_count = generator->next((unsigned char*)ctx->block, TAKE_AMOUNT)) == 0) {
            return;
        }

        cudacover_check(ctx, TAKE_AMOUNT);
        for (int i = 0; i < ctx->results_counter; ++i) {
            Result result = Result((unsigned char*)&ctx->block[ctx->results[i] * featuresCount], featuresCount);
            results->append(std::move(result));
        }
    }

    cudacover_free(&ctx);
}

#else // NOT CUDA

void breadthWorker() {
    uint_fast32_t _count;
    uint_fast8_t _tasks[dataFile.getFeaturesLen() * TAKE_AMOUNT];
    bool coverAll;
    bool workIsEnd;
    uint_fast16_t coverKoef;

    for(;;) {
        workIsEnd = false;

        IF_MULTITHREAD(resultsLock->lock());
        if (results->isFull()) {
            workIsEnd = true;
        }
        IF_MULTITHREAD(resultsLock->unlock());

        if (workIsEnd) {
            return;
        }

        IF_MULTITHREAD(generatorLock->lock());
        _count = generator->next(_tasks, TAKE_AMOUNT);
        IF_MULTITHREAD(generatorLock->unlock());

        if (_count == 0) {
            return;
        }

        for(uint_fast32_t i=0; i<_count; ++i) {
            coverAll = true;

            for (uint_fast32_t r=0; r<dataFile.getUimSetLen(); ++r) {
                coverKoef = 0;

                for(uint_fast16_t j=0; j<dataFile.getFeaturesLen(); ++j) {
                    if (_tasks[i * dataFile.getFeaturesLen() + j] && uim[r * dataFile.getFeaturesLen() + j]) {
                        coverKoef += 1;
                        if (coverKoef == cover) {
                            break;
                        }
                    }
                }

                if (coverKoef < cover) {
                    coverAll = false;
                    break;
                }
            }

            performedTasks += 1;

            if (coverAll) {
                Result result = Result(&_tasks[i * dataFile.getFeaturesLen()], dataFile.getFeaturesLen());

                IF_MULTITHREAD(resultsLock->lock());
                results->append(std::move(result));
                IF_MULTITHREAD(resultsLock->unlock());
            }
        }
    }
}
#endif // NOT CUDA

#endif // BREADTH_ALGO
