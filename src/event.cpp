#include "event.h"

Event::Event(EventType type, int start, int delay, int duration, int id, int trigCode) :
    type_(type), start_(start), delay_(delay), duration_(duration), id_(id), trigCode_(trigCode)
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
