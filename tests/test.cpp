#include <QStringList>
#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include "test.h"
#include "eventreader.h"
#include "eventcontainer.h"
#include "motiondetector.h"
#include "glthread.h"
#include "cycdatabuffer.h"

Test::Test(QObject *parent) : QObject(parent)
{
}

Test::~Test()
{
}

void Test::testEventReader()
{
    EventReader evReader;
    EventContainer<Event*> events;

    QStringList strList;
    strList.append(QString("Event: type=d"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Event: type"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Event: type="));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Object: type=image, id=0, filename=../img/button.png"));
    strList.append(QString("Event: type=image, start=2000, x=0, y=0, objectId=0"));
    QCOMPARE(evReader.loadEvents(strList, events), true);

    strList.clear();
    strList.append(QString("Event: type=flip, start=0, trigcode=10"));
    strList.append(QString("Event: type=fade in, start=1000, duration=5000, delay=0, trigcode=10"));
    strList.append(QString("Event: type=fade out, start=0, duration=5000, delay=0, trigcode=10"));
    strList.append(QString("Event: type=text, start=0, x=0, y=0, color=black, text=test"));
    strList.append(QString("Event: type=rotate, start=99, angle=90"));
    strList.append(QString("Event: type=freeze, start=0, trigcode=10"));
    strList.append(QString("Delete: start=0, id=0, trigcode=3"));
    strList.append(QString("Delete: start=0, type=image, trigcode=43"));
    strList.append(QString("Event: type=zoom, start=0, scale=1.5, duration=2000, trigcode=10"));
    QCOMPARE(evReader.loadEvents(strList, events), true);

    strList.clear();
    strList.append(QString("Object: type=video, id=0, length=2000"));
    strList.append(QString("Event: type=record, start=0, duration=2000, objectId=0"));
    strList.append(QString("Event: type=playback, start=0, duration=2000, objectId=0"));
    QCOMPARE(evReader.loadEvents(strList, events), true);

    strList.clear();
    strList.append(QString("Object: type=video, id=0, filename=../img/"));
    strList.append(QString("Event: type=image, start=2000, x=0, y=0, objectId=0"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Object: type=image, id=1, filename=../img/button.png"));
    strList.append(QString("Event: type=image, start=2000, x=0, y=0, objectId=0"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Object: type=video, id=0, length=1000"));
    strList.append(QString("Event: type=record, start=0, duration=2000, objectId=0"));
    strList.append(QString("Event: type=playback, start=0, duration=2000, objectId=0"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Event: type=zoom, start=0, scale=0.9, duration=2000"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Evnt: type=zoom, start=0, scale=0.9, duration=2000"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Evnt: type=zoom, start=0, scale=0.9, dration=2000"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Event: type=flip, start=das"));
    QCOMPARE(evReader.loadEvents(strList, events), false);

    strList.clear();
    strList.append(QString("Event: type=flip, start=1.2"));
    QCOMPARE(evReader.loadEvents(strList, events), false);
}

void Test::testMotionDetector()
{
    /*
    cv::Mat bg, hand1, hand2;

    bg = cv::imread("../tests/background.ppm");
    hand1 = cv::imread("../tests/hand1.ppm");
    hand2 = cv::imread("../tests/hand2.ppm");
    QCOMPARE(bg.empty(), false);
    QCOMPARE(hand1.empty(), false);
    QCOMPARE(hand2.empty(), false);

    MotionDetector motionDetector;
    motionDetector.updateBackground(bg);
    motionDetector.updateFrame(hand1);
    motionDetector.startTracking();
    motionDetector.updateFrame(hand1);
    QCOMPARE(motionDetector.movementDetected(), false);

    motionDetector.updateFrame(hand2);
    QCOMPARE(motionDetector.movementDetected(), true);
    */
}

QTEST_MAIN(Test)
