#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <common.h>

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

    explicit Event(Event::EventType type, int start, int delay=0, int duration=0, int id = -1, int trigCode = 0);
    virtual ~Event() {}

    int         getStart() const {return start_;}
    int         getDelay() const {return delay_;}
    int         getDuration() const {return duration_;}
    int         getId() const {return id_;}
    EventType   getType() const {return type_;}
    int         getTrigCode() const {return trigCode_;}

    void        appendLog(const QString &str) {log_ = str;}
    QString     getLog() const {return log_;}

    virtual void pause() {}
    virtual void unpause() {}

protected:
    EventType   type_;

    int         start_, delay_, duration_;
    int         id_;
    int         trigCode_;

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
    explicit DelEvent(int start, int delay, int delId, int trigCode = 0);
    explicit DelEvent(int start, int delay, EventType delType, int trigCode = 0);

    EventType getDelType() const {return delType_;}
    int getDelId() const {return delId_;}

private:
    int         delId_;
    EventType   delType_;

};

#endif // EVENT_H
