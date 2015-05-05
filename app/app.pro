include(../defaults.pri)

TEMPLATE = app

QT += opengl
CONFIG += link_pkgconfig

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp


CONFIG(debug, debug|release) {
    BUILD = debug

    #$$BUILD doesn't work for PRE_TARGEDEPS, so it needs to be defined separately for debug and release
    PRE_TARGETDEPS+=../lib/libvidman_debug.a
}

CONFIG(release, debug|release) {
    BUILD = release
    PRE_TARGETDEPS+=../lib/libvidman_release.a

    # remove possible other optimization flags
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2

    # add the desired -O3 if not present
    QMAKE_CXXFLAGS_RELEASE += -O3
}

PKGCONFIG += opencv
LIBS += -L../lib -lvidman_$$BUILD -L/usr/share/lib -ldc1394 -ljpeg

TARGET = vidman_$$BUILD

DESTDIR = ../bin
OBJECTS_DIR = ../build/$$BUILD/app
MOC_DIR = ../build/$$BUILD/app
RCC_DIR = ../build/$$BUILD/app
UI_DIR = ../build/$$BUILD/app
