include(../defaults.pri)
TEMPLATE = app

CONFIG += qtestlib link_pkgconfig

QT += opengl

#INCLUDEPATH += /usr/include/opencv
PKGCONFIG += opencv
LIBS += -L../lib -lvidman_debug -L/usr/share/lib -ldc1394

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    test.cpp

HEADERS += \
    test.h

PRE_TARGETDEPS+=../lib/libvidman_debug.a

TARGET = tests

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests
RCC_DIR = ../build/tests
UI_DIR = ../build/tests
