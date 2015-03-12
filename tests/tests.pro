include(../defaults.pri)
TEMPLATE = app

CONFIG += qtestlib


INCLUDEPATH += /usr/include/opencv
LIBS += -L../src -lvidman
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
    testeventreader.cpp

HEADERS += \
    testeventreader.h
