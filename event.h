#ifndef EVENT_H
#define EVENT_H

#include <cv.h>
#include <highgui.h>
#include <QTimer>
#include <common.h>
#include "timerwithpause.h"

/*!
Event classes are used to process a video frame i.e. add an event to the video.
Event is an abstract base class that should be inherited by all subclasses. Event has
a pure virtual function apply(&frame) which takes a reference to an OpenCV matrice as
a parameter and which should be implemented by subclasses.
  */


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
    EVENT_DETECT_MOTION
};

class Event
{
public:
                Event(EventType type, int start, int delay=0, int duration=0, int id = -1, int trigCode = 0);
    virtual     ~Event();

    virtual void apply(cv::Mat &frame) = 0;

    int         getStart() const;
    int         getDelay() const;
    int         getDuration() const;
    int         getId() const;
    EventType   getType() const;
    int         getTrigCode() const;

    void        appendLog(const QString &str);
    QString     getLog() const;

    virtual void pause();
    virtual void unpause();

protected:
    EventType   type;

    int         start, delay, duration;
    int         id;
    int         trigCode;

    QString     log;
};


class RemoveEvent : public Event
{
public:

    RemoveEvent(int start, int delay, int removeId, int trigCode = 0);
    RemoveEvent(int start, int delay, EventType removeType, int trigCode = 0);

    void apply(cv::Mat &frame);

    EventType getRemoveType() const;
    int getRemoveId() const;

private:
    int         removeId;
    EventType   removeType;

};


class FlipEvent : public Event
{
public:

    FlipEvent(int start, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
};


class FadeInEvent : public QObject, public Event
{
    Q_OBJECT
public:
    FadeInEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
    void pause();
    void unpause();


private:
    TimerWithPause  timerWithPause;
    int             amount, interval;
    bool            stopped;
};

class FadeOutEvent: public QObject, public Event
{
    Q_OBJECT
public:
    FadeOutEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    TimerWithPause  timerWithPause;
    int     amount, interval;
    bool    stopped;
};

class ImageEvent : public Event
{
public:
    ImageEvent(int start, cv::Point2i pos,
               const cv::Mat &image, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
private:

    void overlayImage(const cv::Mat &background, const cv::Mat &foreground,
                                cv::Mat &output, cv::Point2i location);

    cv::Mat     image;
    cv::Point2i pos;
};


class TextEvent : public Event
{
public:
        TextEvent(int start, QString str, cv::Scalar color,
                  cv::Point2i pos, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    cv::Scalar  color;
    cv::Point2i pos;
    QString     str;
};


class RotateEvent : public Event
{
public:
    RotateEvent(int start, int angle, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    int angle;
};


class FreezeEvent: public Event
{
public:
    FreezeEvent(int start, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    bool    started;

    cv::Mat freezedFrame;
};

class DetectMotionEvent: public Event
{
public:
    DetectMotionEvent(int start, int delay, int trigCode = 0);

    void apply(cv::Mat &frame);

};

#endif // EVENT_H
