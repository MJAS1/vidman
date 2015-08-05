#ifndef EventContainer_H
#define EventContainer_H

#include <vector>
#include "event.h"

/*!
Custom container class for storing events.
  */

namespace cv {
class Mat;
}

class EventContainer
{
public:

    EventContainer();

    void            clear();
    void            deleteId(int id);
    void            deleteType(Event::EventType);
    void            append(EventPtr event);
    void            insertSorted(EventPtr event);
    void            applyEvents(cv::Mat& frame);
    void            pauseEvents();
    void            unpauseEvents();

    bool            empty() const;
    int             getTotalDuration() const;

    EventPtr        pop_front();
    const EventPtr& operator[](int id) const;

private:
    EventContainer(const EventContainer&);
    EventContainer& operator=(const EventContainer&);

    std::vector<EventPtr> events_;
};

#endif // EventContainer_H
