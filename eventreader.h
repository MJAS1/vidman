#ifndef EVENTREADER_H
#define EVENTREADER_H

#include <QString>
#include <QStringList>
#include "eventcontainer.h"
#include "imagecontainer.h"

class EventReader
{
public:
    EventReader();

    bool loadEvents(const QStringList &strList, EventContainer *events);

private:
    bool    readEvent(const QString &str, EventContainer *events, int lineNumber);
    bool    readImageObject(const QString &str, int lineNumber);
    bool    readRemoveEvent(const QString &str, EventContainer *events, int lineNumber);

    void    errorMsg(const QString &message) const;
    float   toFloat(const QString &str, int line, const QString &param) const;
    int     toInt(const QString &str, int line, const QString &param) const;

    ImageContainer imageContainer;
};

#endif // EVENTREADER_H
