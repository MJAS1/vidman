#include <QStringList>
#include "testeventreader.h"
#include "eventreader.h"
#include "eventcontainer.h"

TestEventReader::TestEventReader(QObject *parent) : QObject(parent)
{

}

TestEventReader::~TestEventReader()
{

}

void TestEventReader::readEvents()
{
    EventReader evReader;
    EventContainer<Event*> events;

    QStringList strList;
    strList.append(QString("Event: type=d"));
    QCOMPARE(evReader.loadEvents(strList, events), false);
}


QTEST_MAIN(TestEventReader)
