TEMPLATE = app
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    workrow.cpp \
    input_matrix.cpp \
    irreduntant_matrix.cpp \
    row.cpp \
    timecollector.cpp

HEADERS += \
    workrow.h \
    irredundant_matrix.h \
    input_matrix.h \
    row.h \
    timecollector.h

OTHER_FILES +=

