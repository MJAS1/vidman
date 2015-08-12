#ifndef TESTEVENTREADER_H
#define TESTEVENTREADER_H

#include <QtTest>
#include <QObject>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = 0);
    ~Test();

private slots:
    void testEventReader();
    //void testMotionDetector();
};

#endif // TESTEVENTREADER_H
