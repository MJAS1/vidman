# tests.pro
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

CONFIG += link_pkgconfig
QT += opengl

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += testlib
}
else {
    CONFIG+=qtestlib
}

PKGCONFIG += opencv4
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
