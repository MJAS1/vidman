#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <memory>
#include <QObject>
#include <QMap>
#include <opencv2/opencv.hpp>
#include "event.h"
#include "settings.h"


/*! Parse events from a QStringList and stores them in an EventContainer. The
QStringList is created from MainWindow's TextEdit when start is clicked.
LoadEvents returns false if the input text is incorrectly formatted.
*/

using std::shared_ptr;
class EventContainer;

//VideoObject is used to create record and playback events. Contains the frames
//belonging to the particular video and length in milliseconds.
struct VideoObject {
    QList<cv::Mat> frames_;
    int duration_;
};

class EventParser : public QObject
{
    Q_OBJECT
public:
    EventParser();

    bool loadEvents(const QStringList &strList, EventContainer& events, MainWindow* window = nullptr);

signals:
    void error(const QString&) const;

private:
    bool                parseEvent(const QString &str, EventContainer& events, int lineNumber);
    bool                parseObject(const QString &str, int lineNumber);
    bool                parseDelEvent(const QString &str, EventContainer& events, int lineNumber);
    bool                parseEventParam(const QString &param, const QString &value, int lineNumber);
    bool                parseObjectParam(const QString &param, const QString &value, int lineNumber);

    bool                createEvent(EventPtr&, int lineNumber);

    float               toFloat(const QString &str, int line, const QString &param) const;
    int                 toInt(const QString &str, int line, const QString &param) const;

    Settings            settings_;

    int                 start_, duration_, delay_;
    int                 x_, y_, objectId_, eventId_;
    int                 angle_, trigCode_, length_;
    int                 trigCode2_, target_, tolerance_;

    float               scale_;
    bool                objectIdOk_;

    QString             string_, fileName_, objectType_;
    cv::Scalar          color_;
    Event::EventType    type_;

    //VideoObjects shared between a RecordEvent and a PlaybackEvent
    QMap<int, shared_ptr<VideoObject>>  videoObjects_;
    QMap<int, cv::Mat>                  imageObjects_;
};

#endif // EVENTPARSER_H
