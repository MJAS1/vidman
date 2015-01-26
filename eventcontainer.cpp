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
    if(!empty())
    {
        for(Iterator iter = events_.begin(); iter != events_.end(); iter++)
            delete *iter;

        events_.clear();
    }
}

template <typename T>
Event* EventContainer<T>::pop_front()
{
    Event *ev = *events_.begin();
    events_.erase(events_.begin());
    return ev;
}

template <typename T>
void EventContainer<T>::removeId(int id)
{
    Iterator iter = events_.begin();
    while(iter != events_.end())
    {
        if((*iter)->getId() == id)
        {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        iter++;
    }
}

template <typename T>
void EventContainer<T>::removeType(Event::EventType type)
{
    Iterator iter = events_.begin();
    while(iter != events_.end())
    {
        if((*iter)->getType() == type)
        {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        iter++;
    }
}

template <typename T>
bool EventContainer<T>::empty()
{
    return events_.empty();
}

template <typename T>
T EventContainer<T>::operator [](int id) const
{
    return events_[id];
}

template <typename T>
void EventContainer<T>::append(T event)
{
    events_.append(event);
}

template <typename T>
void EventContainer<T>::prepend(T event)
{
    events_.prepend(event);
}

template <typename T>
typename EventContainer<T>::Iterator EventContainer<T>::begin()
{
    return events_.begin();
}

template <typename T>
typename EventContainer<T>::Iterator EventContainer<T>::end()
{
    return events_.end();
}

template <typename T>
typename EventContainer<T>::ConstIterator EventContainer<T>::begin() const
{
    return events_.begin();
}

template <typename T>
typename EventContainer<T>::ConstIterator EventContainer<T>::end() const
{
    return events_.end();
}

template <>
void EventContainer<VideoEvent*>::applyEvents(cv::Mat &frame) const
{
    for(ConstIterator iter = events_.begin(); iter != events_.end(); iter++)
        (*iter)->apply(frame);
}

template <>
void EventContainer<VideoEvent*>::pauseEvents()
{
    for(Iterator iter = events_.begin(); iter != events_.end(); iter++)
        (*iter)->pause();
}

template <>
void EventContainer<VideoEvent*>::unpauseEvents()
{
    for(Iterator iter = events_.begin(); iter != events_.end(); iter++)
        (*iter)->unpause();
}

template class EventContainer<Event*>;
template class EventContainer<VideoEvent*>;
