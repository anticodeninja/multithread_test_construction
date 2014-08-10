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
    irreduntant_matrix.cpp \
    row.cpp \
    timecollector.cpp \
    optimal_plan.cpp \
    fast_plan.cpp

HEADERS += \
    workrow.h \
    irredundant_matrix.h \
    input_matrix.h \
    row.h \
    timecollector.h \
    optimal_plan.h \
    optimal_plan.h \
    global_settings.h \
    fast_plan.h

OTHER_FILES +=

