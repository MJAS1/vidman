#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include "eventcontainer.h"
#include "eventreader.h"
#include "timerwithpause.h"

class VideoEvent;

const int DEFAULT_PRIORITY = 0;
const int FREEZE_PRIORITY = 3;
const int RECORD_PRIORITY = 4;
const int PLAYBACK_PRIORITY = 4;

/*!
Event objects are used to specify the starting time, duration, effects etc.
of events handled by the program.
  */

class Event
{
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
        duration_(duration), id_(id), trigCode_(trigCode), priority_(priority), ready_(false) {}

    virtual ~Event() {}

    virtual void applyFrame(cv::Mat &) {}
    virtual void applyContainer(EventContainer&) {}
    virtual void pause() {}
    virtual void unpause() {}

    int         getStart() const {return start_;}
    int         getDelay() const {return delay_;}
    int         getDuration() const {return duration_;}
    int         getId() const {return id_;}
    EventType   getType() const {return type_;}
    int         getTrigCode() const {return trigCode_;}
    int         getPriority() const {return priority_;}

    bool        isReady() {return ready_;}

    void        appendLog(const QString &str) {log_ = str;}
    QString     getLog() const {return log_;} 

protected:
    EventType   type_;

    int         start_, delay_, duration_;
    int         id_;
    int         trigCode_;
    int         priority_;
    bool        ready_;

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

    void applyContainer(EventContainer &);

private:
    int         delId_;
    EventType   delType_;

};

class FlipEvent : public Event
{
public:

    explicit FlipEvent(int start, int delay, int id = -1, int trigCode = 0);

    virtual void applyFrame(cv::Mat &frame);
    virtual void applyContainer(EventContainer&);
};

class FadeInEvent : public Event
{
public:
    explicit FadeInEvent(int start, int duration = 5, int delay=0, int id = -1,
                         int trigCode = 0);

    virtual void applyFrame(cv::Mat &frame);
    virtual void applyContainer(EventContainer&);
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

    virtual void applyFrame(cv::Mat &frame);
    virtual void applyContainer(EventContainer&);
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

    virtual void applyFrame(cv::Mat &frame);
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

    virtual void applyFrame(cv::Mat &frame);

private:
    cv::Scalar  color_;
    cv::Point2i pos_;
    QString     str_;
};


class RotateEvent : public Event
{
public:
    explicit RotateEvent(int start, int angle, int delay, int id = -1, int trigCode = 0);

    virtual void applyFrame(cv::Mat &frame);
    virtual void applyContainer(EventContainer&);

private:
    int angle_;
};


class FreezeEvent : public Event
{
public:
    explicit FreezeEvent(int start, int delay, int id = -1, int trigCode = 0);

    virtual void applyFrame(cv::Mat &frame);
    virtual void applyContainer(EventContainer&);

private:
    bool    started_;

    cv::Mat freezedFrame_;
};

class ZoomEvent : public Event
{
public:
    explicit ZoomEvent(int start, double scale, int duration = 5, int delay = 0,
                       int id = -1, int trigCode = 0);

    virtual void applyFrame(cv::Mat &frame);
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

    virtual void applyFrame(cv::Mat &frame);
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

    virtual void applyFrame(cv::Mat &frame);
    void pause();
    void unpause();

private:
    VideoPtr video_;
    QList<cv::Mat>::Iterator iter_;
    bool finished_;
    bool paused_;
};

#endif // EVENT_H
