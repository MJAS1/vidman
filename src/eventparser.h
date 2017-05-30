#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <memory>
#include <QObject>
#include <QMap>
#include <opencv2/opencv.hpp>
#include "event.h"
#include "settings.h"

using std::shared_ptr;
class EventContainer;

/*
 * VideoObject is used to create record and playback events. Contains the frames
 * belonging to the particular video and length in milliseconds.
 */
struct VideoObject {
    QList<cv::Mat> frames_;
    int duration_;
};

struct EventAttributes {
    int start_ = 0;
    int duration_ = 0;
    int delay_ = 0;
    int x_ = 0;
    int y_ = 0;
    int objectId_ = 0;
    int eventId_ = -1;
    int angle_ = 0;
    int trigCode_ = 0;
    int trigCode2_ = 0;
    int target_ = 0;
    int tolerance_ = 0;
    int axis_ = 1;
    float scale_ = 1;
    bool objectIdOk_ = false;
    QString string_;
    QString fileName_;
    QString objectType_;
    cv::Scalar color_ = cv::Scalar(0, 0, 0);
    Event::EventType type_ = Event::EVENT_NULL;
};

/*!
 * Parses events from a QStringList and stores them in an EventContainer. The
 * QStringList is created from MainWindow's TextEdit when start is clicked.
 * LoadEvents returns false if the input text is incorrectly formatted.
*/
class EventParser : public QObject
{
    Q_OBJECT
public:
    bool loadEvents(const QStringList &strList, EventContainer &events,
                    MainWindow* window = nullptr);

signals:
    void error(const QString&) const;

private:
    bool parseEvent(const QString &str, EventContainer& events) const;
    bool parseObject(const QString &str);
    bool parseDelEvent(const QString &str, EventContainer& events) const;
    bool parseEventParam(const QString &param, const QString &value,
                         EventAttributes &evAttr) const;
    bool parseObjectParam(const QString &param, const QString &value,
                          EventAttributes &evAttr) const;

    EventPtr createEvent(const EventAttributes &evAttr) const;
    EventPtr createDelEvent(EventAttributes &evAttr) const;

    float toFloat(const QString &str, const QString &param) const;
    int toInt(const QString &str, const QString &param) const;

    Settings settings_;
    int currentLine_;

    //VideoObjects shared between a RecordEvent and a PlaybackEvent
    QMap<int, shared_ptr<VideoObject>>  videoObjects_;
    QMap<int, cv::Mat>                  imageObjects_;
};

#endif // EVENTPARSER_H
