#include "eventcontainer.h"
#include "videoevent.h"

template <typename T>
EventContainer<T>::EventContainer()
{
}

template <typename T>
EventContainer<T>::~EventContainer()
{
    clear();
}

template <typename T>
void EventContainer<T>::clear()
{
    if(!this->empty())
    {
        for(Iterator iter = events.begin(); iter != events.end(); iter++)
            delete *iter;

        events.clear();
    }
}

template <typename T>
Event* EventContainer<T>::pop_front()
{
    Event *ev = *events.begin();
    events.erase(events.begin());
    return ev;
}

template <typename T>
void EventContainer<T>::removeId(int id)
{
    Iterator iter = events.begin();
    while(iter != events.end())
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

template <typename T>
void EventContainer<T>::removeType(EventType type)
{
    Iterator iter = events.begin();
    while(iter != events.end())
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

template <typename T>
bool EventContainer<T>::empty()
{
    return events.empty();
}

template <typename T>
T EventContainer<T>::operator [](int id) const
{
    return events[id];
}

template <typename T>
void EventContainer<T>::append(T event)
{
    events.append(event);
}

template <typename T>
void EventContainer<T>::prepend(T event)
{
    events.prepend(event);
}

template <typename T>
typename EventContainer<T>::Iterator EventContainer<T>::begin()
{
    return events.begin();
}

template <typename T>
typename EventContainer<T>::Iterator EventContainer<T>::end()
{
    return events.end();
}

template <typename T>
typename EventContainer<T>::ConstIterator EventContainer<T>::begin() const
{
    return events.begin();
}

template <typename T>
typename EventContainer<T>::ConstIterator EventContainer<T>::end() const
{
    return events.end();
}

template class EventContainer<Event*>;
template class EventContainer<VideoEvent*>;
