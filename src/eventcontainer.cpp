#include <numeric>
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
    if(!empty()) {
        for(Iterator iter = events_.begin(); iter != events_.end(); ++iter)
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
void EventContainer<T>::deleteId(int id)
{
    Iterator iter = events_.begin();
    while(iter != events_.end()) {
        if((*iter)->getId() == id) {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        ++iter;
    }
}

template <typename T>
void EventContainer<T>::deleteType(Event::EventType type)
{
    Iterator iter = events_.begin();
    while(iter != events_.end()) {
        if((*iter)->getType() == type) {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        ++iter;
    }
}

template <typename T>
bool EventContainer<T>::empty() const
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

template <>
void EventContainer<VideoEvent*>::insertSorted(VideoEvent *event)
{
    auto iter = std::lower_bound(events_.begin(), events_.end(), event,
                        [](const VideoEvent* l, const VideoEvent* r) {
                            return l->getPriority() > r->getPriority();
                        });
    events_.insert(iter, event);
}

template <>
void EventContainer<VideoEvent*>::insert(Event *event)
{
    //Remove duplicate events of certain event types to prevent the program from
    //slowing down because of applying the same event multiple times
    switch (event->getType()) {
    case Event::EVENT_FLIP:
        deleteType(Event::EVENT_FLIP);
        break;

    case Event::EVENT_FADEIN:
    case Event::EVENT_FADEOUT:
        deleteType(Event::EVENT_FADEIN);
        deleteType(Event::EVENT_FADEOUT);
        break;

    case Event::EVENT_ROTATE:
        deleteType(Event::EVENT_ROTATE);
        break;

    case Event::EVENT_FREEZE:
        deleteType(Event::EVENT_FREEZE);
        break;

    case Event::EVENT_REMOVE:
        if(event->getType())
            deleteType(static_cast<DelEvent*>(event)->getDelType());
        else
            deleteId(static_cast<DelEvent*>(event)->getDelId());

        delete event;
        return;

    default:
        break;
    }

    insertSorted(static_cast<VideoEvent*>(event));
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
    for(ConstIterator iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->apply(frame);
}

template <typename T>
void EventContainer<T>::pauseEvents()
{
    for(Iterator iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->pause();
}

template <typename T>
void EventContainer<T>::unpauseEvents()
{
    for(Iterator iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->unpause();
}

template <typename T>
int EventContainer<T>::getTotalDuration() const
{
    return std::accumulate(begin(), end(), 0,
                           [](int x, const T& a) {
                                return x + a->getStart() + a->getDelay();
                            });
}

template class EventContainer<Event*>;
template class EventContainer<VideoEvent*>;
