#ifndef TESTEVENTPARSER_H
#define TESTEVENTPARSER_H

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
