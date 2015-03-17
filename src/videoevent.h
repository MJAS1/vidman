#ifndef VIDEOEVENT_H
#define VIDEOEVENT_H

#include <memory>
#include <QList>
#include "event.h"
#include "timerwithpause.h"

/*!
VideoEvents are used to process a video frame. VideoEvent is an abstract base class
that should be inherited by all subclasses. Event has a pure virtual function apply(&frame)
which takes a reference to an OpenCV matrice as a parameter and which should be implemented
by subclasses.
*/

class VideoEvent : public Event
{
public:
    explicit VideoEvent(EventType type, int start, int delay, int duration, int id, int trigCode, int priority = 0);
    virtual     ~VideoEvent();

    virtual void apply(cv::Mat &frame) = 0;

    int getPriority() const {return priority_;}

private:
    int priority_;
};

inline bool compareEventPriorities(const VideoEvent* left, const VideoEvent* right)
{
    return left->getPriority() > right->getPriority();
}

class FlipEvent : public VideoEvent
{
public:

    explicit FlipEvent(int start, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
};


class FadeInEvent : public VideoEvent
{
public:
    explicit FadeInEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();


private:
    TimerWithPause  timerWithPause_;
    int             amount_, interval_;
    bool            stopped_;
};

class FadeOutEvent: public VideoEvent
{
public:
    explicit FadeOutEvent(int start, int duration = 5, int delay=0, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    TimerWithPause  timerWithPause_;
    int     amount_, interval_;
    bool    stopped_;
};

class ImageEvent : public VideoEvent
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


class TextEvent : public VideoEvent
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


class RotateEvent : public VideoEvent
{
public:
    explicit RotateEvent(int start, int angle, int delay, int id = -1, int trigCode = 0);

    virtual void apply(cv::Mat &frame);

private:
    int angle_;
};


class FreezeEvent : public VideoEvent
{
public:
    explicit FreezeEvent(int start, int delay, int id = -1, int trigCode = 0, int priority = 3);

    virtual void apply(cv::Mat &frame);

private:
    bool    started_;

    cv::Mat freezedFrame_;
};

class ZoomEvent : public VideoEvent
{
public:
    explicit ZoomEvent(int start, double scale, int duration = 5, int delay = 0, int id = -1, int trigCode = 0);

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

class RecordEvent : public VideoEvent
{
public:
    typedef typename std::shared_ptr<QList<cv::Mat>> FramesPtr;

    explicit RecordEvent(int start, FramesPtr frames, int delay = 0, int duration = 1000, int id = -1, int trigCode = 0, int priority = 4);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();
private:
    TimerWithPause timer_;
    FramesPtr frames_;
    bool finished_;
    bool paused_;
};

class PlaybackEvent : public VideoEvent
{
public:
    typedef typename std::shared_ptr<QList<cv::Mat>> FramesPtr;

    explicit PlaybackEvent(int start, FramesPtr frames, int delay = 0, int duration = 1000, int id = -1, int trigCode = 0, int priority = 4);

    virtual void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    FramesPtr frames_;
    QList<cv::Mat>::Iterator iter_;
    bool finished_;
    bool paused_;
};

#endif // VIDEOEVENT_H