/*
 * event.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QObject>
#include <QList>
#include <memory>
#include <QPixmap>
#include <opencv2/opencv.hpp>

#include "common.h"
#include "timerwithpause.h"

class EventContainer;
struct VideoObject;
struct Frame;
class MainWindow;

/*!
 * Event objects are used to specify the starting time, duration, effects,
 * trigger code, etc. of video events handled by the program. Each event can
 * modify an OpenCV Mat video frame or an EventContainer holding other events by
 * using the apply() virtual functions.
 *
 * The apply(EventContainer&) function can be used, for example, to remove
 * other events from the container. This is used by DelEvents and other events
 * for which only one event of the type should exist inside the container.
 *
 * When apply(frame) is used for the first time, the event will emit a
 * triggered() signal as long as the derived class calls Event::apply(frame) in
 * the overrided function.
 */

class Event;
typedef typename std::unique_ptr<Event> EventPtr;

class Event : public QObject
{
    Q_OBJECT
public:

    // All possible event types
    enum EventType
    {
        EVENT_NULL,
        EVENT_FLIP,
        EVENT_FADEIN,
        EVENT_FADEOUT,
        EVENT_IMAGE,
        EVENT_TEXT,
        EVENT_DELETE,
        EVENT_ROTATE,
        EVENT_FREEZE,
        EVENT_ZOOM,
        EVENT_RECORD,
        EVENT_PLAYBACK,
        EVENT_DETECT_MOTION
    };

    // Priorities are used to determine the order in which to apply events.
    enum Priority
    {
        DEFAULT_PRIORITY = 0,
        IMAGE_PRIORITY = 1,
        FADE_PRIORITY = 2,
        FREEZE_PRIORITY = 3,
        RECORD_PRIORITY = 4,
        PLAYBACK_PRIORITY = 4,
        MOTION_DETECTOR_PRIORITY = 5
    };

    explicit Event(Event::EventType type=EVENT_NULL, int start=0, int delay=0,
                   int duration=0, int id = -1, uint8_t trigCode = 0,
                   Priority priority = DEFAULT_PRIORITY) :
        type_(type), start_(start), delay_(delay), duration_(duration), id_(id),
        trigCode_(trigCode), priority_(priority), ready_(false), first_(true) {}

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
    void        triggered(uint8_t trigCode_, const QString& log);

protected:
    EventType   type_;

    int         start_;
    int         delay_;
    int         duration_;
    int         id_;
    int         trigCode_;
    Priority    priority_;
    bool        ready_;
    bool        first_;
    QString     log_;

private:
    Event(const Event&);
    Event& operator=(const Event&);
};

/*!
 * This class is used to specify what event should be deleted. When
 * apply(EventContainer&) is called, either an event with delId_ is deleted or
 * all events of deType_.
 */
class DelEvent : public Event
{
public:

    /*
     * DelEvent can be initialized to remove either an event with a specific
     * id or all the events of a given type.
     */
    explicit DelEvent(int start, int delay, int delId, uint8_t trigCode=0) :
        Event(EVENT_DELETE, start, delay, 0, -1, trigCode),
        delId_(delId), tag_(ID) {}

    explicit DelEvent(int start, int delay, EventType delType, uint8_t trigCode=0):
        Event(EVENT_DELETE, start, delay, 0, -1, trigCode),
        delType_(delType), tag_(TYPE) {}

    void apply(EventContainer &);

private:
    union {
        int         delId_;
        EventType   delType_;
    };
    enum {ID, TYPE} tag_;
};

class FlipEvent : public Event
{
public:

    explicit FlipEvent(int start, int axis=1, int delay=0, int id=-1,
                       uint8_t trigCode=0);

    virtual void apply(cv::Mat &frame);
    virtual void apply(EventContainer&);

private:
    /*
     * Specifies how to flip.
     * 0: Flip around x axis,
     * positive value e.g. 1: Flip around y axis,
     * negative value e.g. -1: Flip around both axes
     */
    int axis_;
};

/*!
 * This event fades the video view to a black screen.
 */
class FadeOutEvent: public Event
{
public:
    explicit FadeOutEvent(int start, int duration=5, int delay=0, int id = -1,
                          uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
    void apply(EventContainer&);
    void pause();
    void unpause();

private:
    TimerWithPause  timerWithPause_;

    int             amount_;
    int             interval_;
    bool            stopped_;
};

/*!
 * This event fades the video view back to normal from black screen.
 */
class FadeInEvent : public Event
{
public:
    explicit FadeInEvent(int start, int duration=5, int delay=0, int id=-1,
                         uint8_t trigCode=0);

    void apply(cv::Mat &frame);
    void apply(EventContainer&);
    void pause();
    void unpause();


private:
    TimerWithPause  timerWithPause_;

    int             amount_;
    int             interval_;
    bool            stopped_;
};

class ImageEvent : public Event
{
public:
    explicit ImageEvent(int start, const cv::Point2i& pos,
                        const cv::Mat &image, int delay, int id = -1,
                        uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
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
                       const cv::Point2i& pos, int delay, int id = -1,
                       uint8_t trigCode = 0);

    void apply(cv::Mat &frame);

private:
    cv::Scalar  color_;
    cv::Point2i pos_;
    QString     str_;
};

class RotateEvent : public Event
{
public:
    explicit RotateEvent(int start, int angle, int delay, int id = -1,
                         uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
    void apply(EventContainer&);

private:
    int angle_;
};

class FreezeEvent : public Event
{
public:
    explicit FreezeEvent(int start, int delay, int id = -1,
                         uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
    void apply(EventContainer&);

private:
    bool    started_;

    cv::Mat frame_;
};

class ZoomEvent : public Event
{
public:
    explicit ZoomEvent(int start, double scale, int duration = 5, int delay = 0,
                       int id = -1, uint8_t trigCode = 0);

    void apply(cv::Mat &frame);

private:
    double scale_;
};

class RecordEvent : public Event
{
public:
    typedef typename std::shared_ptr<VideoObject> VideoPtr;

    explicit RecordEvent(int start, VideoPtr video, int delay = 0,
                         int duration = 1000, int id = -1,
                         uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
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
                           int duration = 1000, int id = -1, uint8_t trigCode = 0);

    void apply(cv::Mat &frame);
    void pause();
    void unpause();

private:
    VideoPtr video_;
    QList<Frame>::Iterator iter_;
    bool finished_;
    bool paused_;
};

/*!
 * This event detects movement between subsequent frames. It stores three
 * frames: previous, current and next, and detects motion using "differential
 * images" method (https://blog.cedric.ws/opencv-simple-motion-detection).
 *
 * After movement has finished, the duration from event onset to movement end
 * is drawn as feedback to the subsequent frames. Depending on the target_ and
 * tolerance_  variables, the time is drawn either in green if the duration is
 * within limits (succesfull movement) or in red if not (failed movement). The
 * event emits the succesCode if the movement succeeded and the failCode
 * otherwise.
 *
 * This class can also emit a QPixmpap of the differential image with the
 * movement shown as white pixels. The emitted pixmap can then be drawn to a
 * MotionDialog.
 *
 * Currently movement is only detected from a ROI where the right hand was
 * situated during the MEG experiments. This may have to be changed in the
 * future for different experiments.
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
                                 int delay, int id, uint8_t trigCode,
                                 uint8_t successCode, uint8_t failCode,
                                 int x = VIDEO_WIDTH/2-30,
                                 int y = VIDEO_HEIGHT/2,
                                 State state = WAITING);

    explicit MotionDetectorEvent();

    void apply(cv::Mat &frame);
    void apply(EventContainer&);

signals:
    void            pixmapReady(const QPixmap&);
    void            priorityChanged();

private:
    // How long the the feedback duration is shown (in ms).
    static const int TextDuration = 1000;

    // Minimum duration of no motion to consider movement as finished (in ms).
    static const int MinStopTime = 100;

    void            createMotionPixmap();
    int             nChanges();

    void            waiting();
    void            tracking();
    void            maybeFinished();
    void            finished(cv::Mat &frame);

    State           state_;
    QElapsedTimer   movementTimer_, finishTimer_;
    qint64          time_;

    cv::Mat         prev_, current_, next_, result_;
    cv::Scalar      color_;
    cv::Point       textPos_;
    cv::Rect        roi_;

    bool            motionDialog_;

    /*
     * Threshold value for the number of differing pixels to consider movement
     * as started.
     */
    int             threshold_;
    uint8_t         successCode_;
    uint8_t         failCode_;
    int             target_;
    int             tolerance_;
};

/*!
 * This event can be used to pause the running script. The triggered() signal
 * is connected to MainWindow's pause() slot by EventParser.
 */
class PauseEvent : public Event
{
    Q_OBJECT
public:
    PauseEvent();

    void apply(cv::Mat &);
    void unpause();

private:
    cv::Point txtPos1_, txtPos2_;
};

#endif // EVENT_H
