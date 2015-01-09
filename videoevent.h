#ifndef VIDEOEVENT_H
#define VIDEOEVENT_H

#include "event.h"

/*!
VideoEvents are used to process a video frame. VideoEvent is an abstract base class
that should be inherited by all subclasses. Event has a pure virtual function apply(&frame)
which takes a reference to an OpenCV matrice as a parameter and which should be implemented
by subclasses.
*/

class VideoEvent : public Event
{
public:
    explicit VideoEvent(EventType type, int start, int delay, int duration, int id, int trigCode);
    virtual     ~VideoEvent();

    virtual void apply(cv::Mat &frame) = 0;
};

class FlipEvent : public VideoEvent
{
public:

    explicit FlipEvent(int start, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
};


class FadeInEvent : public VideoEvent
{
public:
    explicit FadeInEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
    void pause();
    void unpause();


private:
    TimerWithPause  timerWithPause;
    int             amount, interval;
    bool            stopped;
};

class FadeOutEvent: public VideoEvent
{
public:
    explicit FadeOutEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    TimerWithPause  timerWithPause;
    int     amount, interval;
    bool    stopped;
};

class ImageEvent : public VideoEvent
{
public:
    explicit ImageEvent(int start, const cv::Point2i& pos,
               const cv::Mat &image, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);
private:

    void overlayImage(const cv::Mat &background, const cv::Mat &foreground,
                                cv::Mat &output, const cv::Point2i& location);

    cv::Mat     image;
    cv::Point2i pos;
};


class TextEvent : public VideoEvent
{
public:
    explicit TextEvent(int start, const QString& str, cv::Scalar color,
                  const cv::Point2i& pos, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    cv::Scalar  color;
    cv::Point2i pos;
    QString     str;
};


class RotateEvent : public VideoEvent
{
public:
    explicit RotateEvent(int start, int angle, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    int angle;
};


class FreezeEvent: public VideoEvent
{
public:
    explicit FreezeEvent(int start, int delay, int id = -1, int trigCode = 0);

    void apply(cv::Mat &frame);

private:
    bool    started;

    cv::Mat freezedFrame;
};


#endif // VIDEOEVENT_H
