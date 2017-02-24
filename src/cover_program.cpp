#include <algorithm>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <chrono>
#include <deque>
#include <vector>
#include <thread>

#include "timecollector.h"
#include "resultset.h"

#include "cover_generator.h"
#include "cover_depth_task.h"

uint_fast8_t cover;
uint_fast32_t rowsCount;
uint_fast16_t featuresCount;

uint_fast8_t* uim;
uint_fast32_t performedTasks;
ResultSet* results;
std::mutex* resultsLock;

uint_fast16_t costBarrier;

const uint_fast32_t TAKE_AMOUNT = 1024;
CoverGenerator* generator;
std::mutex* generatorLock;

void readFile(const char* filename);

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

int main(int argc, const char** argv)
{    
    TimeCollector::Initialize();
    TimeCollector::ThreadInitialize();
    TimeCollectorEntry executionTime(Counters::All);

    readFile(argv[0]);
    cover = atoi(argv[1]);

#ifdef MULTITHREAD
    resultsLock = new std::mutex();
#endif

    results = new ResultSet(atoi(argv[2]));
    performedTasks = 0;

#if defined(DEPTH_ALGO)
    findInDepth();
#elif defined(BREADTH_ALGO)
    findInBreadth();
#else
    #error Cover algo is not chosen
#endif

    std::cout << *results << std::endl;

    executionTime.Stop();

    delete results;

#ifdef MULTITHREAD
    delete resultsLock;
#endif

    TimeCollector::ThreadFinalize();
    std::ofstream timeCollectorOutput("current_profile.txt");
    TimeCollector::PrintInfo(timeCollectorOutput);
    
    return 0;
}

#endif

void readFile(const char* filename) {
    START_COLLECT_TIME(readingInput, Counters::ReadingInput);
    
    std::ifstream input_stream(filename);
    input_stream >> rowsCount;
    input_stream >> featuresCount;
    uim = new uint_fast8_t[rowsCount * featuresCount];

    int temp;
    for(uint_fast32_t i=0; i<rowsCount; ++i) {
        for(uint_fast16_t j=0; j<featuresCount; ++j) {
            input_stream >> temp;
            uim[i * featuresCount + j] = !!temp;
        }
    }
    
    STOP_COLLECT_TIME(readingInput);
}

#ifdef DEPTH_ALGO
void findInDepth() {
    costBarrier = UINT_FAST16_MAX;

    uint_fast32_t weights[featuresCount];
    std::tuple<uint_fast32_t, uint_fast16_t> weights_sorted[featuresCount];

    for(uint_fast16_t i=0; i<featuresCount; ++i) {
        weights[i] = 0;
    }
    for(uint_fast32_t i=0; i<rowsCount; ++i) {
        for(uint_fast16_t j=0; j<featuresCount; ++j) {
            weights[j] += uim[i * featuresCount + j];
        }
    }

    for(uint_fast16_t i=0; i<featuresCount; ++i) {
        weights_sorted[i] = std::make_tuple(weights[i], i);
    }

    std::sort(&weights_sorted[0], &weights_sorted[featuresCount]);

#ifdef MULTITHREAD
    std::vector<std::thread> threads(featuresCount);
    std::vector<std::deque<CoverDepthTask*>> tasks(featuresCount);
    
    for(uint_fast16_t threadId = 0; threadId < featuresCount; ++threadId) {
        tasks[threadId].push_front(CoverDepthTask::createInitTask(threadId, uim, rowsCount, featuresCount));
        START_COLLECT_TIME(threading, Counters::Threading);
        threads[threadId] = std::thread([threadId, &tasks]()
        {
            TimeCollector::ThreadInitialize();
            depthWorker(tasks[threadId]);
            TimeCollector::ThreadFinalize();
        });
        STOP_COLLECT_TIME(threading);
    }
    
    for(uint_fast16_t threadId = 0; threadId < featuresCount; ++threadId) {
        threads[threadId].join();
    }
#else
    std::deque<CoverDepthTask*> tasks;
    for(uint_fast16_t i=0; i<featuresCount; ++i) {
        tasks.push_front(CoverDepthTask::createInitTask(i, uim, rowsCount, featuresCount));
    }
    depthWorker(tasks);
#endif
}

void depthWorker(std::deque<CoverDepthTask*>& tasks) {
    uint_fast32_t index = 0;
    uint_fast8_t* rows[rowsCount];
    uint_fast8_t covering[rowsCount];
    uint_fast8_t mask[featuresCount];
    uint_fast32_t weights[featuresCount];
    std::tuple<uint_fast32_t, uint_fast16_t> weights_sorted[featuresCount];
    CoverDepthTask* task;
    
    for(;;) {
        if (tasks.size() == 0) {
            return;
        }

        task = tasks.front();
        tasks.pop_front();

        Result result = Result(task->getMask(), featuresCount);

        if (result.getCost() >= costBarrier) {
            delete task;
            continue;
        }

        performedTasks += 1;

        if (task->getRowsCount() == 0) {
#ifdef MULTITHREAD
            resultsLock->lock();
#endif
            if (results->append(std::move(result)) != ResultSet::IGNORED) {
                if (results->isFull()) {
                    costBarrier = results->get(results->getSize() - 1).getCost();
                }
            }
#ifdef MULTITHREAD
            resultsLock->unlock();
#endif
            delete task;
            continue;
        }

        for(uint_fast16_t i=0; i<featuresCount; ++i) {
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

        for(uint_fast16_t i=0; i<featuresCount; ++i) {
            weights[i] = 0;
        }
        for(uint_fast32_t i=0; i<index; ++i) {
            for(uint_fast16_t j=0; j<featuresCount; ++j) {
                weights[j] += rows[i][j];
            }
        }

        for(uint_fast16_t i=0; i<featuresCount; ++i) {
            weights_sorted[i] = std::make_tuple(weights[i], i);
        }

        std::sort(&weights_sorted[0], &weights_sorted[featuresCount]);

        for(uint_fast16_t i=0; i<featuresCount; ++i) {
            if (!mask[i]) {
                tasks.push_front(new CoverDepthTask(i, featuresCount, mask, index, rows, covering));
            }
        }

        delete task;
    }
}
#endif

#ifdef BREADTH_ALGO
void findInBreadth() {
    generator = new CoverGenerator(featuresCount);
    
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

void breadthWorker() {
    uint_fast32_t _count;
    uint_fast8_t _tasks[featuresCount * TAKE_AMOUNT];
    bool coverAll;
    bool workIsEnd;
    uint_fast16_t coverKoef;
    
    for(;;) {
        workIsEnd = false;
#ifdef MULTITHREAD
        resultsLock->lock();
#endif
        if (results->isFull()) {
            workIsEnd = true;
        }
#ifdef MULTITHREAD
        resultsLock->unlock();
#endif

        if (workIsEnd) {
            return;
        }
        
#ifdef MULTITHREAD
        generatorLock->lock();
#endif
        _count = generator->next(_tasks, TAKE_AMOUNT);
#ifdef MULTITHREAD
        generatorLock->unlock();
#endif

        if (_count == 0) {
            return;
        }

        for(uint_fast32_t i=0; i<_count; ++i) {
            coverAll = true;

            for (uint_fast32_t r=0; r<rowsCount; ++r) {
                coverKoef = 0;

                for(uint_fast16_t j=0; j<featuresCount; ++j) {
                    if (_tasks[i * featuresCount + j] && uim[r * featuresCount + j]) {
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
                Result result = Result(&_tasks[i * featuresCount], featuresCount);

#ifdef MULTITHREAD
                resultsLock->lock();
#endif
                results->append(std::move(result));
#ifdef MULTITHREAD
                resultsLock->unlock();
#endif
            }
        }
    }
}
#endif
