#ifndef EventContainer_H
#define EventContainer_H

#include <QList>
#include "event.h"
/*!
Custom template class for storing events. Should only store pointers to
Event class or subclass objects. Handles memory deallocation.
  */

class VideoEvent;

template <typename T>
class EventContainer
{
public:

    typedef typename QList<T>::Iterator Iterator;
    typedef typename QList<T>::ConstIterator ConstIterator;

    EventContainer();
    ~EventContainer();

    void    clear();
    void    deleteId(int id);
    void    deleteType(Event::EventType);
    void    append(T event);
    void    prepend(T event);
    void    insertSorted(VideoEvent* event);
    void    applyEvents(cv::Mat& frame) const;
    void    pauseEvents();
    void    unpauseEvents();

    bool    empty() const;
    int     getTotalDuration() const;

    Iterator      begin();
    Iterator      end();
    ConstIterator begin() const;
    ConstIterator end() const;

    Event*  pop_front();

    T operator [](int id) const;

private:
    EventContainer(const EventContainer&);
    EventContainer& operator=(const EventContainer&);

    QList<T> events_;
};


#endif // EventContainer_H
