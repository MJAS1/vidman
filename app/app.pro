include(../defaults.pri)

TEMPLATE = app

QT += opengl
TARGET = vidman

QMAKE_CXXFLAGS += -std=c++11

# remove possible other optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

# add the desired -O3 if not present
QMAKE_CXXFLAGS_RELEASE *= -O3

SOURCES += \
    main.cpp

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
LIBS += -ljpeg

Release:DESTDIR = release

Debug:DESTDIR = debug
