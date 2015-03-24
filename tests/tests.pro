include(../defaults.pri)
TEMPLATE = app

CONFIG += qtestlib
CONFIG +=link_pkgconfig

CONFIG(debug, debug|release) {
    BUILD = debug
}

CONFIG(release, debug|release) {
    BUILD = release
}

#INCLUDEPATH += /usr/include/opencv
PKGCONFIG += opencv
LIBS += -L../lib -lvidman_$$BUILD
LIBS += -L/usr/share/lib -ldc1394
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    test.cpp

HEADERS += \
    test.h

TARGET = tests_$$BUILD

DESTDIR = ../bin
OBJECTS_DIR = ../build/$$BUILD/tests
MOC_DIR = ../build/$$BUILD/tests
RCC_DIR = ../build/$$BUILD/tests
UI_DIR = ../build/$$BUILD/tests
