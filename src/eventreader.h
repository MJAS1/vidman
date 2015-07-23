#ifndef EVENTREADER_H
#define EVENTREADER_H

#include <memory>
#include <QObject>
#include <QMap>
#include <opencv2/opencv.hpp>
#include "eventcontainer.h"
#include "settings.h"


/*! Reads events from a QStringList and stores them in an eventcontainer. The
QStringList is created from MainWindow's TextEdit when start is clicked.
LoadEvents returns false if the input text is incorrectly forcv::Matted.
*/

using std::shared_ptr;

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

    bool loadEvents(const QStringList &strList, EventContainer<Event*>& events);

signals:
    void error(const QString&) const;

private:
    bool    readEvent(const QString &str, EventContainer<Event*>& events, int lineNumber);
	bool	readObject(const QString &str, int lineNumber);
    bool    readDelEvent(const QString &str, EventContainer<Event*>& events, int lineNumber);

    float   toFloat(const QString &str, int line, const QString &param) const;
    int     toInt(const QString &str, int line, const QString &param) const;

    //VideoObjects shared between a RecordEvent and a PlaybackEvent
    QMap<int, shared_ptr<VideoObject>> videoObjects_;
    QMap<int, cv::Mat> imageObjects_;

    Settings settings_;
};

#endif // EVENTREADER_H
