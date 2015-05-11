cmake_minimum_required(VERSION 3.2)
project(multithread)

ADD_DEFINITIONS(-std=c++11)
ADD_DEFINITIONS(-lpthread)

ADD_DEFINITIONS(-DTIME_PROFILE)
ADD_DEFINITIONS(-DDEBUG_MODE)

set(SOURCE_FILES
    multithread/fast_plan.cpp
    multithread/fast_plan.h
    multithread/global_settings.h
    multithread/input_matrix.cpp
    multithread/input_matrix.h
    multithread/irredundant_matrix_array.cpp
    multithread/irredundant_matrix_array.h
    multithread/irredundant_matrix_base.cpp
    multithread/irredundant_matrix_base.h
    multithread/irredundant_matrix_queue.h
    multithread/irreduntant_matrix_factory.h
    multithread/irreduntant_matrix_queue.cpp
    multithread/main.cpp
    multithread/optimal_plan.cpp
    multithread/optimal_plan.h
    multithread/row.cpp
    multithread/row.h
    multithread/timecollector.cpp
    multithread/timecollector.h
    multithread/workrow.cpp
    multithread/workrow.h)

add_executable(multithread ${SOURCE_FILES})