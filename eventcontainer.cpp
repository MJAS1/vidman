#include "eventcontainer.h"

EventContainer::EventContainer()
{
}

EventContainer::EventContainer(const EventContainer &other)
{
    events = other.events;
}

EventContainer& EventContainer::operator =(const EventContainer &other)
{
    if(events == other.events)
        return *this;

    clear();
    events = other.events;

    return *this;
}

EventContainer::~EventContainer()
{
    clear();
}

Event*& EventContainer::operator [](const int i)
{
    return events[i];
}

bool EventContainer::empty() const
{
    return events.empty();
}

size_t EventContainer::size() const
{
    return events.size();
}

void EventContainer::clear()
{
    if(!events.empty())
    {
        for(size_t i = 0; i < events.size(); i++)
            delete events[i];

        events.clear();
    }
}

void EventContainer::push_back(Event *ev)
{
    events.push_back(ev);
}

Event* EventContainer::pop_front()
{
    Event *ev = events[0];
    events.erase(events.begin());
    return ev;
}

void EventContainer::removeId(int id)
{
    for(QVector<Event*>::iterator iter = events.begin(); iter != events.end();)
    {
        if((*iter)->getId() == id)
        {
            delete (*iter);
            iter = events.erase(iter);
            continue;
        }
        iter++;
    }
}

void EventContainer::removeType(EventType type)
{
    for(QVector<Event*>::iterator iter = events.begin(); iter != events.end();)
    {
        if((*iter)->getType() == type)
        {
            delete (*iter);
            iter = events.erase(iter);
            continue;
        }
        iter++;
    }
}

