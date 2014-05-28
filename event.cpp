#include "event.h"

Event::Event(EventType type, int start, int delay, int duration, int id, int trigCode) :
    type(type), start(start), delay(delay), duration(duration), id(id), trigCode(trigCode)
{
}

Event::~Event()
{

}

int Event::getStart() const
{
    return start;
}


int Event::getDelay() const
{
    return delay;
}

int Event::getDuration() const
{
    return duration;
}

Event::EventType Event::getType() const
{
    return type;
}

int Event::getId() const
{
    return id;
}

void Event::appendLog(const QString &str)
{
    log = str;
}

QString Event::getLog() const
{
    return log;
}

int Event::getTrigCode() const
{
    return trigCode;
}

void Event::pause()
{
}

void Event::unpause()
{
}

RemoveEvent::RemoveEvent(int start, int delay, int removeId, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeId(removeId), removeType(EVENT_NULL)
{
}

RemoveEvent::RemoveEvent(int start, int delay, EventType removeType, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeType(removeType)
{
}

Event::EventType RemoveEvent::getRemoveType() const
{
    return removeType;
}

int RemoveEvent::getRemoveId() const
{
    return removeId;
}

