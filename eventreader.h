#ifndef EVENTREADER_H
#define EVENTREADER_H

#include <memory>
#include <QString>
#include <QStringList>
#include "eventcontainer.h"
#include "imagecontainer.h"
#include "settings.h"

/*! Reads events from a QStringList and stores them in an eventcontainer. The
QStringList is created from MainWindow's TextEdit when start is clicked.
LoadEvents returns false if the input text is incorrectly formatted.
*/

class EventReader : public QObject
{
    Q_OBJECT
public:
    EventReader();

    bool loadEvents(const QStringList &strList, EventContainer<Event*>& events);

signals:
    void error(const QString&) const;

private:
    bool    readEvent(const QString &str, EventContainer<Event*>& events, int lineNumber);
    bool    readImageObject(const QString &str, int lineNumber);
    bool    readVideoObject(const QString &str, int lineNumber);
    bool    readRemoveEvent(const QString &str, EventContainer<Event*>& events, int lineNumber);

    float   toFloat(const QString &str, int line, const QString &param) const;
    int     toInt(const QString &str, int line, const QString &param) const;

    ImageContainer imageContainer_;
    QMap<int, std::shared_ptr<QList<cv::Mat>>> videoObjects_;

    Settings settings_;
};

#endif // EVENTREADER_H
