#ifndef EventContainer_H
#define EventContainer_H

#include <QVector>
#include "event.h"

/*!
Custom vector class for storing events. Handles memory deallocation.
  */

class EventContainer : public QVector<Event*>
{
public:
    EventContainer();
    ~EventContainer();

    void    clear();
    void    removeId(int id);
    void    removeType(EventType);

    Event*  pop_front();

private:
    //Rule of Three
    EventContainer(const EventContainer& other);
    EventContainer& operator=(const EventContainer& other);
};

#endif // EventContainer_H
