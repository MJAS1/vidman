#ifndef EventContainer_H
#define EventContainer_H

#include <QList>
#include "event.h"

/*!
Custom template class for storing events. Should only store pointers to
Event class or subclass objects. Handles memory deallocation.
  */

template <typename T>
class EventContainer
{
public:

    typedef typename QList<T>::Iterator Iterator;
    typedef typename QList<T>::ConstIterator ConstIterator;

    EventContainer();
    ~EventContainer();

    void    clear();
    void    removeId(int id);
    void    removeType(Event::EventType);
    void    append(T event);
    void    prepend(T event);
    bool    empty();

    Iterator      begin();
    Iterator      end();
    ConstIterator begin() const;
    ConstIterator end() const;

    Event*  pop_front();

    T operator [](int id) const;



private:

    QList<T> events;

    EventContainer(const EventContainer& other);
    EventContainer& operator=(const EventContainer& other);
};


#endif // EventContainer_H
