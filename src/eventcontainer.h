#ifndef EventContainer_H
#define EventContainer_H

#include <QList>

/*!
Custom template class for storing events. Should only store pointers to
Event class or subclass objects. Handles memory deallocation.
  */

class VideoEvent;
class Event;
namespace cv {
    class Mat;
}

class EventContainer
{
public:

    typedef typename QList<Event*>::Iterator Iterator;
    typedef typename QList<Event*>::ConstIterator ConstIterator;

    EventContainer();
    ~EventContainer();

    void    clear();
    void    deleteId(int id);
    void    deleteType(int);
    void    append(Event* event);
    void    prepend(Event* event);
    void    insert(Event* event);
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

    Event* operator [](int id) const;

private:
    EventContainer(const EventContainer&);
    EventContainer& operator=(const EventContainer&);

    void    insertSorted(Event* event);

    QList<Event*> events_;
};


#endif // EventContainer_H
