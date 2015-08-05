#ifndef EVENTREADER_H
#define EVENTREADER_H

#include <memory>
#include <QObject>
#include <QMap>
#include <opencv2/opencv.hpp>
#include "event.h"
#include "settings.h"


/*! Reads events from a QStringList and stores them in an EventContainer. The
QStringList is created from MainWindow's TextEdit when start is clicked.
LoadEvents returns false if the input text is incorrectly formatted.
*/

using std::shared_ptr;
class EventContainer;

//VideoObject is used to create record and playback events. Contains the frames
//belonging to the particular video and length in milliseconds.
struct VideoObject {
    QList<cv::Mat> frames_;
    int length_;
};

class EventReader : public QObject
{
    Q_OBJECT
public:
    EventReader();

    bool loadEvents(const QStringList &strList, EventContainer& events);

signals:
    void error(const QString&) const;

private:
    bool                readEvent(const QString &str, EventContainer& events, int lineNumber);
    bool                readObject(const QString &str, int lineNumber);
    bool                readDelEvent(const QString &str, EventContainer& events, int lineNumber);
    bool                readEventParam(const QString &param, const QString &value, int lineNumber);
    bool                readObjectParam(const QString &param, const QString &value, int lineNumber);

    bool                createEvent(EventPtr&, int lineNumber);

    float               toFloat(const QString &str, int line, const QString &param) const;
    int                 toInt(const QString &str, int line, const QString &param) const;

    Settings            settings_;

    int                 start_, duration_, delay_;
    int                 x_, y_, objectId_, eventId_;
    int                 angle_, trigCode_, length_;

    float               scale_;
    bool                objectIdOk_;

    QString             text_, fileName_, objectType_;
    cv::Scalar          color_;
    Event::EventType    type_;

    //VideoObjects shared between a RecordEvent and a PlaybackEvent
    QMap<int, shared_ptr<VideoObject>>  videoObjects_;
    QMap<int, cv::Mat>                  imageObjects_;
};

#endif // EVENTREADER_H
