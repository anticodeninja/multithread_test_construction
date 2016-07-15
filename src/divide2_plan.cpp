#include "divide2_plan.h"

#include <iostream>
#include <limits>

#include "global_settings.h"

Divide2Plan::Divide2Plan(int* counts, int len)
{    
    auto step = 0;
    _tasks.push_back(std::vector<Divide2Task>());
    _tasks[step].push_back(Divide2Task());

    auto& task0 = _tasks[step][0];
    for (auto i=0; i<len; ++i) {
        task0.append(i, counts[i]);
    }

    auto width = !task0.isEmpty() ? 1 : 0;
    _width = width;
    
    for (;;) {
        auto hasFuture = false;
        _tasks.push_back(std::vector<Divide2Task>());

        step += 1;
        width = 0;
#ifndef MULTITHREAD_DIVIDE2_OPTIMIZED
        for (auto chunk=0; chunk < _tasks[step-1].size(); ++chunk) {
            auto& parent = _tasks[step - 1][chunk];
                
            _tasks[step].push_back(Divide2Task());
            _tasks[step].push_back(Divide2Task());
            
            auto& task1 = _tasks[step][2*chunk+0];
            for (auto i=0; i<parent.getFirstSize(); ++i) {
                task1.append(parent.getFirst(i), counts[parent.getFirst(i)]);
            }

            auto& task2 = _tasks[step][2*chunk+1];
            for (auto i=0; i<parent.getSecondSize(); ++i) {
                task2.append(parent.getSecond(i), counts[parent.getSecond(i)]);
            }

            width += 2;
            hasFuture = hasFuture || !task1.isEmpty() || !task2.isEmpty();
        }
#else
        for (auto chunk=0; chunk < _tasks[step-1].size(); ++chunk) {
            auto& parent = _tasks[step - 1][chunk];
            if (parent.isEmpty()) {
                continue;
            }

            if (parent.getFirstSize() > 1) {
                _tasks[step].push_back(Divide2Task());
                auto& task = _tasks[step][width];
                for (auto i=0; i<parent.getFirstSize(); ++i) {
                    task.append(parent.getFirst(i), counts[parent.getFirst(i)]);
                }
                width += 1;
                hasFuture = true;
            }

            if (parent.getSecondSize() > 1) {
                _tasks[step].push_back(Divide2Task());
                auto& task = _tasks[step][width];
                for (auto i=0; i<parent.getSecondSize(); ++i) {
                    task.append(parent.getSecond(i), counts[parent.getSecond(i)]);
                }
                width += 1;
                hasFuture = true;
            }
        }
#endif

        if (width > _width) {
            _width = width;
        }

        if (!hasFuture) {
            break;
        }
    }

    _steps = step + 1;

    DEBUG_BLOCK (
       getDebugStream() << "Divide2Plan: " << std::endl;
       
       for (auto step = 0; step < _steps; ++step) {
           getDebugStream() << step << "| ";
           for (auto chunk=0; chunk < _tasks[step].size(); ++chunk) {
               for (auto i = 0; i < _tasks[step][chunk].getFirstSize(); ++i) {
                   getDebugStream() << _tasks[step][chunk].getFirst(i) << " ";
               }
               
               getDebugStream() << "- ";

               for (auto i = 0; i < _tasks[step][chunk].getSecondSize(); ++i) {
                   getDebugStream() << _tasks[step][chunk].getSecond(i) << " ";
               }

               getDebugStream() << "| ";
           }
           getDebugStream() << std::endl;
       }
    )
}
