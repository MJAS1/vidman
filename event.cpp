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

RemoveEvent::RemoveEvent(int start, int delay, int removeId, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeId_(removeId), removeType_(EVENT_NULL)
{
}

RemoveEvent::RemoveEvent(int start, int delay, EventType removeType, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeType_(removeType)
{
}

Event::EventType RemoveEvent::getRemoveType() const
{
    return removeType_;
}

int RemoveEvent::getRemoveId() const
{
    return removeId_;
}
