TEMPLATE = app
CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS+= -pthread -std=c++11
QMAKE_LFLAGS +=  -pthread -std=c++11

SOURCES += main.cpp \
    workrow.cpp \
    input_matrix.cpp \
    row.cpp \
    timecollector.cpp \
    optimal_plan.cpp \
    fast_plan.cpp \
    irredundant_matrix_array.cpp \
    irreduntant_matrix_queue.cpp \
    irredundant_matrix_base.cpp

HEADERS += \
    workrow.h \
    input_matrix.h \
    row.h \
    timecollector.h \
    optimal_plan.h \
    optimal_plan.h \
    global_settings.h \
    fast_plan.h \
    irredundant_matrix_base.h \
    irredundant_matrix_array.h \
    irredundant_matrix_queue.h

OTHER_FILES +=

