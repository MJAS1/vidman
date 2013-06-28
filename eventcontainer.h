#ifndef EventContainer_H
#define EventContainer_H

#include <QVector>
#include "event.h"

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
    EventContainer(const EventContainer& other);
    EventContainer& operator=(const EventContainer& other);
};

#endif // EventContainer_H
