#ifndef EventContainer_H
#define EventContainer_H

#include <vector>
#include "event.h"

/*!
 * Custom container class for storing events.
 */

namespace cv {
class Mat;
}

class EventContainer : public QObject
{
    Q_OBJECT
public:
    EventContainer(QObject *parent = nullptr);

    void            clear();
    void            deleteId(int id);
    void            deleteType(Event::EventType);
    void            append(EventPtr event);

    //Insert a new event and sort according to priority.
    void            insertSorted(EventPtr event);
    void            applyEvents(cv::Mat& frame);
    void            pauseEvents();
    void            unpauseEvents();

    bool            empty() const;
    int             getTotalDuration() const;

    EventPtr        pop_front();
    const EventPtr& operator[](int id) const;

public slots:
    // Sort container according to event priority.
    void            sort();

private:
    std::vector<EventPtr> events_;
};

#endif // EventContainer_H
