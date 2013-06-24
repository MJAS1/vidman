#include "eventcontainer.h"

EventContainer::EventContainer()
{
}



EventContainer::~EventContainer()
{
    clear();
}

void EventContainer::clear()
{
    if(!empty())
    {
        for(iterator iter = begin(); iter != end(); iter++)
            delete *iter;

        QVector<Event*>::clear();
    }
}

Event* EventContainer::pop_front()
{
    Event *ev = *begin();
    erase(begin());
    return ev;
}

void EventContainer::removeId(int id)
{
    for(QVector<Event*>::iterator iter = begin(); iter != end();)
    {
        if((*iter)->getId() == id)
        {

            delete (*iter);
            iter = erase(iter);
            continue;
        }
        iter++;
    }
}

void EventContainer::removeType(EventType type)
{
    for(QVector<Event*>::iterator iter = begin(); iter != end();)
    {
        if((*iter)->getType() == type)
        {
            delete (*iter);
            iter = erase(iter);
            continue;
        }
        iter++;
    }
}

