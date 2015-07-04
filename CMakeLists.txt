cmake_minimum_required(VERSION 2.8)
project(multithread)

ADD_DEFINITIONS(-std=c++11)


OPTION(IRREDUNTANT_VECTOR "Use vector for keeping work-matrix in memory" OFF)
IF(IRREDUNTANT_VECTOR)
    ADD_DEFINITIONS(-DIRREDUNTANT_VECTOR)
ENDIF(IRREDUNTANT_VECTOR)

OPTION(TIME_PROFILE "Use time-collector for profiling perfomance" OFF)
IF(TIME_PROFILE)
    ADD_DEFINITIONS(-DTIME_PROFILE)
ENDIF(TIME_PROFILE)

OPTION(DIFFERENT_MATRICES "Use differrent matrices for merging" OFF)
IF(DIFFERENT_MATRICES)
    ADD_DEFINITIONS(-DDIFFERENT_MATRICES)
ENDIF(DIFFERENT_MATRICES)

OPTION(DEBUG_MODE "Use additional messages to finding errors" OFF)
IF(DEBUG_MODE)
    ADD_DEFINITIONS(-DDEBUG_MODE)
ENDIF(DEBUG_MODE)

OPTION(MULTITHREAD "Use multithread realization of algorithm" OFF)
IF(MULTITHREAD)
    ADD_DEFINITIONS(-DMULTITHREAD)
ENDIF(MULTITHREAD)


set(SOURCE_FILES
    multithread/fast_plan.cpp
    multithread/fast_plan.h
    multithread/global_settings.h
    multithread/input_matrix.cpp
    multithread/input_matrix.h
    multithread/irredundant_matrix.h
    multithread/irreduntant_matrix.cpp
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

target_link_libraries(multithread -pthread)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)