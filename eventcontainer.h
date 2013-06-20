#ifndef EventContainer_H
#define EventContainer_H

#include <QVector>
#include "event.h"

class EventContainer
{
public:
    EventContainer();
    EventContainer(const EventContainer& other);
    EventContainer& operator=(const EventContainer& other);

    ~EventContainer();

    bool    empty() const;
    size_t  size() const;
    void    push_back(Event* ev);
    void    clear();

    void    removeId(int id);
    void    removeType(EventType);

    Event*  pop_front();

    Event*& operator[](const int i);

private:
    QVector<Event*> events;
};

#endif // EventContainer_H
