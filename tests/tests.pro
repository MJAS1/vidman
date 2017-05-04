include(../defaults.pri)
TEMPLATE = app

CONFIG += link_pkgconfig
QT += opengl

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += testlib
}
else {
    CONFIG+=qtestlib
}

PKGCONFIG += opencv
LIBS += -L../lib -lvidman_debug -ldc1394 -ljpeg

QMAKE_CXXFLAGS += -std=c++11

SOURCES += test.cpp

HEADERS += test.h

PRE_TARGETDEPS+=../lib/libvidman_debug.a

TARGET = tests

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests
RCC_DIR = ../build/tests
UI_DIR = ../build/tests
