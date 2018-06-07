/*
 * test.cpp
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

#include <QStringList>
#include <QTest>
#include <QSignalSpy>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "config.h"
#include "camera.h"
#include "cameraworker.h"
#include "test.h"
#include "eventparser.h"
#include "eventcontainer.h"
#include "cycdatabuffer.h"

void Test::testCamera() const
{
    Camera camera;
    camera.setGain(10);
    QCOMPARE(camera.getGain(), 10+GAIN_OFFSET);
    camera.setShutter(10);
    QCOMPARE(camera.getShutter(), 10+SHUTTER_OFFSET);
    camera.setWhiteBalance(10+WHITEBALANCE_OFFSET);
    QCOMPARE(camera.getWhiteBalance(), 10+WHITEBALANCE_OFFSET);

    cv::Mat frame;
    camera >> frame;
    QCOMPARE(frame.empty(), false);
}

void Test::testCameraWorker() const
{
    Camera camera;
    CycDataBuffer cycBuf(CIRC_VIDEO_BUFF_SZ);
    CameraWorker camWorker(&cycBuf, camera);

    QSignalSpy spy(&cycBuf, SIGNAL(chunkReady(unsigned char*)));
    for(int i = 0; i < 5; ++i) camWorker.captureFrame();
    QCOMPARE(spy.count(), 5);
}

void Test::testEventParser() const
{
    EventParser evReader;
    connect(&evReader, SIGNAL(error(const QString&)), this,
            SLOT(errorMsg(const QString&)));
    EventContainer events;

    QFETCH(QStringList, strList);
    QFETCH(bool, result);

    QCOMPARE(evReader.loadEvents(strList, events), result);
}

void Test::testEventParser_data()
{
    QTest::addColumn<QStringList>("strList");
    QTest::addColumn<bool>("result");

    QStringList strList;
    strList << "Event: type=d";
    QTest::newRow("1") << strList << false;

    strList.clear();
    strList << "Event: type";
    QTest::newRow("2") << strList << false;

    strList.clear();
    strList << "Event: type=";
    QTest::newRow("3") << strList << false;

    strList.clear();
    strList << "Object: type=image, id=0, filename=../img/button.png"
            << "Event: type=image, start=2000, x=0, y=0, object_id=0";
    QTest::newRow("4") << strList << true;

    strList.clear();
    strList << "Event: type=flip, start=0, trig_code=10"
            << "Event: type=fade_in, start=1000, duration=5000, delay=0, trig_code=10"
            << "Event: type=fade_out, start=0, duration=5000, delay=0, trig_code=10"
			<< "Event: type=text, start=0, x=0, y=0, color=black, string=test"
			<< "Event: type=rotate, start=99, angle=90"
            << "Event: type=freeze, start=0, trig_code=10"
            << "Delete: start=0, id=0, trig_code=3"
            << "Delete: start=0, type=image, trig_code=43"
            << "Event: type=zoom, start=0, scale=1.5, duration=2000, trig_code=10";
    QTest::newRow("5") << strList << true;

    strList.clear();
    strList << "Object: type=video, id=0, duration=2000"
            << "Event: type=record, start=0, duration=2000, object_id=0"
            << "Event: type=playback, start=0, duration=2000, object_id=0";
    QTest::newRow("6") << strList << true;

    strList.clear();
    strList << "Object: type=video, id=0, filename=../img/"
            << "Event: type=image, start=2000, x=0, y=0, object_id=0";
    QTest::newRow("6") << strList << false;

    strList.clear();
    strList << "Object: type=image, id=1, filename=../img/button.png"
            << "Event: type=image, start=2000, x=0, y=0, object_id=0";
    QTest::newRow("7") << strList << false;

    strList.clear();
    strList << "Object: type=video, id=0, length=1000"
            << "Event: type=record, start=0, duration=2000, object_id=0"
            << "Event: type=playback, start=0, duration=2000, object_id=0";
    QTest::newRow("8") << strList << false;

    strList.clear();
    strList << "Event: type=zoom, start=0, scale=0.9, duration=2000";
    QTest::newRow("9") << strList << false;

    strList.clear();
    strList.append(QString("Evnt: type=zoom, start=0, scale=0.9, duration=2000"));
    QTest::newRow("10") << strList << false;

    strList.clear();
    strList << "Evnt: type=zoom, start=0, scale=0.9, dration=2000";
    QTest::newRow("11") << strList << false;

    strList.clear();
    strList << "Event: type=flip, start=das";
    QTest::newRow("12") << strList << false;

    strList.clear();
    strList << "Event: type=flip, start=1.2";
    QTest::newRow("13") << strList << false;
}

/*
void Test::testMotionDetector()
{

    cv::Mat hand1, hand2;

    hand1 = cv::imread("../tests/hand1.ppm");
    hand2 = cv::imread("../tests/hand2.ppm");
    QCOMPARE(hand1.empty(), false);
    QCOMPARE(hand2.empty(), false);

    MotionDetector motionDetector;
    motionDetector.movementDetected(hand1);
    motionDetector.startTracking(EventPtr(new Event));
    QCOMPARE(motionDetector.movementDetected(hand1), false);
    QCOMPARE(motionDetector.movementDetected(hand2), true);
}
*/

void Test::errorMsg(const QString &s) const
{
    qDebug() << s;
}

QTEST_MAIN(Test)
