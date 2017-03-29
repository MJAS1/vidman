#ifndef TESTEVENTPARSER_H
#define TESTEVENTPARSER_H

#include <QtTest>
#include <QObject>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = 0);
    ~Test();

private slots:
    void testEventParser() const;
    void errorMsg(const QString&) const;

    //void testMotionDetector();
};

#endif // TESTEVENTPARSER_H
