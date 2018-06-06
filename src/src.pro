# src.pro
#
# Author: Manu Sutela
# Copyright (C) 2018 Aalto University
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include(../defaults.pri)

TEMPLATE = lib

QT += core gui opengl

CONFIG += staticlib link_pkgconfig

SOURCES += mainwindow.cpp \
    videodialog.cpp \
    videofilewriter.cpp \
    videocompressorthread.cpp \
    stoppablethread.cpp \
    settings.cpp \
    filewriter.cpp \
    cycdatabuffer.cpp \
    eventcontainer.cpp \
    eventparser.cpp \
    linenumberarea.cpp \
    textedit.cpp \
    highlighter.cpp \
    glvideowidget.cpp \
    timerwithpause.cpp \
    outputdevice.cpp \
    camera.cpp \
    logfile.cpp \
    event.cpp \
    motiondialog.cpp \
    glworker.cpp \
    cameraworker.cpp \

HEADERS += mainwindow.h \
    videodialog.h \
    videofilewriter.h \
    videocompressorthread.h \
    stoppablethread.h \
    settings.h \
    filewriter.h \
    cycdatabuffer.h \
    config.h \
    common.h \
    event.h \
    eventcontainer.h \
    eventparser.h \
    linenumberarea.h \
    textedit.h \
    highlighter.h \
    glvideowidget.h \
    timerwithpause.h \
    outputdevice.h \
    camera.h \
    logfile.h \
    motiondialog.h \
    glworker.h \
    cameraworker.h \

FORMS += mainwindow.ui \
    videodialog.ui \
    motiondialog.ui

PKGCONFIG += opencv
LIBS += -ldc1394 -ljpeg

QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) {
    BUILD = debug
}

CONFIG(release, debug|release) {
    BUILD = release

    # remove possible other optimization flags
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2

    # add the desired -O3 if not present
    QMAKE_CXXFLAGS_RELEASE += -O3
}

TARGET = vidman_$$BUILD

DESTDIR = ../lib
OBJECTS_DIR = ../build/$$BUILD/src
MOC_DIR = ../build/$$BUILD/src
RCC_DIR = ../build/$$BUILD/src
UI_DIR = ../build/$$BUILD/src
