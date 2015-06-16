#include "event.h"

Event::Event(EventType type, int start, int delay, int duration, int id, int trigCode) :
    type_(type), start_(start), delay_(delay), duration_(duration), id_(id), trigCode_(trigCode)
{
}

Event::~Event()
{
}

int Event::getStart() const
{
    return start_;
}

int Event::getDelay() const
{
    return delay_;
}

int Event::getDuration() const
{
    return duration_;
}

Event::EventType Event::getType() const
{
    return type_;
}

int Event::getId() const
{
    return id_;
}

void Event::appendLog(const QString &str)
{
    log_ = str;
}

QString Event::getLog() const
{
    return log_;
}

int Event::getTrigCode() const
{
    return trigCode_;
}

void Event::pause()
{
}

void Event::unpause()
{
}

DelEvent::DelEvent(int start, int delay, int delId, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), delId_(delId), delType_(EVENT_NULL)
{
}

DelEvent::DelEvent(int start, int delay, EventType delType, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), delType_(delType)
{
}

Event::EventType DelEvent::getDelType() const
{
    return delType_;
}

int DelEvent::getDelId() const
{
    return delId_;
}
