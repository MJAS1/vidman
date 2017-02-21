#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QObject>
#include <QList>
#include <memory>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include "timerwithpause.h"

class EventContainer;
class MainWindow;

const int DEFAULT_PRIORITY = 0;
const int FADE_PRIORITY = 1;
const int FREEZE_PRIORITY = 3;
const int RECORD_PRIORITY = 4;
const int PLAYBACK_PRIORITY = 4;
const int MOTION_DETECTOR_PRIORITY = 5;

struct VideoObject;

/*!
Event objects are used to specify the starting time, duration, effects etc.
of events handled by the program. Each event can modify an OpenCV Mat video frame
or an EventContainer holding other events by using the apply virtual functions.
apply(EventContainer&) is used to remove other events from the container for
example by DelEvents and other events for which only one event of the type
should exist in the container.
 */

class Event;
typedef typename std::unique_ptr<Event> EventPtr;

class Event : public QObject
{
    Q_OBJECT
public:

    //All possible events
    enum EventType
    {
        EVENT_NULL,
        EVENT_FLIP,
        EVENT_FADEIN,
        EVENT_FADEOUT,
        EVENT_IMAGE,
        EVENT_TEXT,
        EVENT_REMOVE,
        EVENT_ROTATE,
        EVENT_FREEZE,
        EVENT_ZOOM,
        EVENT_RECORD,
        EVENT_PLAYBACK,
        EVENT_DETECT_MOTION
    };

    explicit Event(Event::EventType type=EVENT_NULL, int start=0, int delay=0,
                   int duration=0, int id = -1, int trigCode = 0, int priority = DEFAULT_PRIORITY) :
        type_(type), start_(start), delay_(delay),
        duration_(duration), id_(id), trigCode_(trigCode), priority_(priority),
        ready_(false), first_(true) {}

    virtual ~Event() {}

    virtual void apply(cv::Mat &);
    virtual void apply(EventContainer&) {}
    virtual void pause() {}
    virtual void unpause() {}

    int         getStart() const {return start_;}
    int         getDelay() const {return delay_;}
    int         getDuration() const {return duration_;}
    int         getId() const {return id_;}
    EventType   getType() const {return type_;}
    int         getPriority() const {return priority_;}

    bool        isReady() const {return ready_;}

    void        appendLog(const QString &str) {log_ = str;}

signals:
    void        triggered(int trigCode_, const QString& log);

protected:
    EventType   type_;

    int         start_, delay_, duration_;
    int         id_;
    int         trigCode_;
    int         priority_;
    bool        ready_;
    bool        first_;

    QString     log_;

private:
    Event(const Event&);
    Event& operator=(const Event&);
};

//This class is used to specify what event should be deleted and when
class DelEvent : public Event
{
public:

    //Remove event can be initialized to remove either an event with a specific id
    //or all the events of a given type
    explicit DelEvent(int start, int delay, int delId, int trigCode = 0) :
        Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), delId_(delId),
        delType_(EVENT_NULL) {}

    explicit DelEvent(int start, int delay, EventType delType, int trigCode = 0):
        Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), delType_(delType)
    {}

    void apply(EventContainer &);

private:
    int         delId_;
    EventType   delType_;

};

class FlipEvent : public Event
{
public:

    explicit FlipEvent(int start, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);
};

class FadeInEvent : public Event
{
public:
    explicit FadeInEvent(int start, int duration = 5, int delay=0, int id = -1,
                         int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);
    void pause();
    void unpause();


private:
    TimerWithPause  timerWithPause_;
    int             amount_, interval_;
    bool            stopped_;
};

class FadeOutEvent: public Event
{
public:
    explicit FadeOutEvent(int start, int duration = 5, int delay=0, int id = -1,
                          int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);
    void pause();
    void unpause();

private:
    TimerWithPause  timerWithPause_;
    int     amount_, interval_;
    bool    stopped_;
};

class ImageEvent : public Event
{
public:
    explicit ImageEvent(int start, const cv::Point2i& pos,
                        const cv::Mat &image, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
private:

    void overlayImage(const cv::Mat &background, const cv::Mat &foreground,
                      cv::Mat &output, const cv::Point2i& location);

    cv::Mat     image_;
    cv::Point2i pos_;
};


class TextEvent : public Event
{
public:
    explicit TextEvent(int start, const QString& str, cv::Scalar color,
                       const cv::Point2i& pos, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);

private:
    cv::Scalar  color_;
    cv::Point2i pos_;
    QString     str_;
};


class RotateEvent : public Event
{
public:
    explicit RotateEvent(int start, int angle, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);

private:
    int angle_;
};


class FreezeEvent : public Event
{
public:
    explicit FreezeEvent(int start, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);

private:
    bool    started_;

    cv::Mat frame_;
};

class ZoomEvent : public Event
{
public:
    explicit ZoomEvent(int start, double scale, int duration = 5, int delay = 0,
                       int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    TimerWithPause timer_;
    double scale_;
    double interval_;
    double coef_;
    bool stopped_;
};

class RecordEvent : public Event
{
public:
    typedef typename std::shared_ptr<VideoObject> VideoPtr;

    explicit RecordEvent(int start, VideoPtr video, int delay = 0,
                         int duration = 1000, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();
private:
    TimerWithPause timer_;
    VideoPtr video_;
    bool finished_;
    bool paused_;
};

class PlaybackEvent : public Event
{
public:
    typedef typename std::shared_ptr<VideoObject> VideoPtr;

    explicit PlaybackEvent(int start, VideoPtr video, int delay = 0,
                           int duration = 1000, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    VideoPtr video_;
    QList<cv::Mat>::Iterator iter_;
    bool finished_;
    bool paused_;
};

/*This class detects movement between subsequent frames. It stores three frames:
 * previous, current and next, and detects motion using "differential images"
 * method https://blog.cedric.ws/opencv-simple-motion-detection. This class can
 * also emit a QPixmpap of a black-and-white image with the movement shown as white
 * pixels. The emitted pixmap can then be drawn to MotionDialog.
*/
class MotionDetectorEvent : public Event
{
    Q_OBJECT
public:
    enum State {
        WAITING,
        TRACKING,
        MAYBE_FINISHED,
        FINISHED,
        MOTION_DIALOG
    };

    explicit MotionDetectorEvent(int start, int target, int tolerance,
                                 int delay, int id, int trigCode, int trigCode2,
                                 State state = WAITING);

    explicit MotionDetectorEvent(State state = MOTION_DIALOG);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);

signals:
    void            pixmapReady(const QPixmap&);
    void            priorityChanged();

private:
    void            createMotionPixmap();
    int             nChanges();

    State           state_;
    QElapsedTimer   movementTimer_, finishTimer_;
    qint64          time_;

    cv::Mat         prev_, current_, next_, result_;
    cv::Point       centroid_;
    cv::Scalar      color_;

    bool            motionDialog_;
    int             threshold_;
    int             trigCode2_;
    int             target_;
    int             tolerance_;
    int             min_x, max_x, min_y, max_y;
};

class PauseEvent : public Event
{
    Q_OBJECT
public:
    explicit PauseEvent(MainWindow* window);

    virtual void apply(cv::Mat &);
    virtual void unpause();

private:
    cv::Point point1, point2;
};

#endif // EVENT_H
