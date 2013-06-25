#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <cv.h>
#include <highgui.h>
#include <QTimer>
#include <string>

enum EventType
{
    EVENT_NULL,
    EVENT_FLIP,
    EVENT_FADEIN,
    EVENT_FADEOUT,
    EVENT_BLACK_SCREEN,
    EVENT_IMAGE,
    EVENT_TEXT,
    EVENT_REMOVE,
    EVENT_ROTATE,
    EVENT_FREEZE
};

class Event
{
public:
                Event(EventType type, float start, float delay=0, float duration=0, int id = -1);
    virtual     ~Event();

    virtual void apply(cv::Mat &frame) = 0;

    float       getStart() const;
    float       getDelay() const;
    float       getDuration() const;

    int         getId() const;
    EventType   getType() const;

protected:
    EventType   type;

    float       start, delay, duration;
    int         id;
};


class RemoveEvent : public Event
{
public:

    RemoveEvent(float start, float delay, int removeId);

    RemoveEvent(float start, float delay, EventType removeType);

    void apply(cv::Mat &frame);

    EventType getRemoveType() const;
    int getRemoveId() const;

private:
    int removeId;
    EventType removeType;

};


class FlipEvent : public Event
{
public:

    FlipEvent(float start, float delay, int id = -1);

    void apply(cv::Mat &frame);
};


class FadeInEvent : public QObject, public Event
{
    Q_OBJECT
public:
    FadeInEvent(float start, float duration = 5, float delay=0, int id = -1);

    void apply(cv::Mat &frame);

private slots:
    void increaseAmount();

private:
    QTimer* timer;
    int     amount;
    bool    stopped;
};

class FadeOutEvent: public QObject, public Event
{
    Q_OBJECT
public:
    FadeOutEvent(float start, float duration = 5, float delay=0, int id = -1);

    void apply(cv::Mat &frame);

private slots:
    void decreaseAmount();

private:
    QTimer* timer;
    int     amount;
    bool    stopped;

};

class ImageEvent : public Event
{
public:
    ImageEvent(float start, cv::Point2i pos,
                    const cv::Mat &image, float delay, int id = -1);

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
        TextEvent(float start, QString str,cv::Point2i pos, float delay, int id = -1);

    void apply(cv::Mat &frame);

private:
    cv::Point2i pos;
    QString     str;
};


class RotateEvent : public Event
{
public:
    RotateEvent(float start, int angle, float delay, int id = -1);

    void apply(cv::Mat &frame);

private:
    int angle;
};


class FreezeEvent: public Event
{
public:
    FreezeEvent(float start, float delay, int id = -1);

    void apply(cv::Mat &frame);

private:
    bool    started;

    cv::Mat freezedFrame;
};

#endif // EVENT_H
