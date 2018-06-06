/*
 * test.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEST_H
#define TEST_H

#include <QtTest>
#include <QObject>

/*! Unit tests */

class Test : public QObject
{
    Q_OBJECT

private slots:
    void testCamera() const;
    void testCameraWorker() const;
    void testEventParser() const;
    void testEventParser_data();
    void errorMsg(const QString&) const;

    //void testMotionDetector();
};

#endif // TESTEVENTPARSER_H
