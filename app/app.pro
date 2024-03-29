# app.pro
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

TEMPLATE = app

QT += opengl
CONFIG += link_pkgconfig

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp

CONFIG(debug, debug|release) {
    BUILD = debug

    #$$BUILD doesn't work for PRE_TARGETDEPS, so it needs to be defined separately for debug and release
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

PKGCONFIG += opencv4
LIBS += -L../lib -lvidman_$$BUILD -ldc1394 -ljpeg

TARGET = vidman_$$BUILD

DESTDIR = ../bin
OBJECTS_DIR = ../build/$$BUILD/app
MOC_DIR = ../build/$$BUILD/app
RCC_DIR = ../build/$$BUILD/app
UI_DIR = ../build/$$BUILD/app
