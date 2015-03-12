#ifndef TESTEVENTREADER_H
#define TESTEVENTREADER_H

#include <QtTest>
#include <QObject>

class TestEventReader : public QObject
{
    Q_OBJECT
public:
    explicit TestEventReader(QObject *parent = 0);
    ~TestEventReader();

private slots:
    void readEvents();
};

#endif // TESTEVENTREADER_H
