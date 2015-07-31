#include <numeric>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "eventcontainer.h"
#include "event.h"

EventContainer::EventContainer()
{
}

EventContainer::~EventContainer()
{
    clear();
}

void EventContainer::clear()
{
    qDeleteAll(events_);
    events_.clear();
}

Event* EventContainer::pop_front()
{
    Event *ev = *events_.begin();
    events_.erase(events_.begin());
    return ev;
}

void EventContainer::deleteId(int id)
{
    auto iter = events_.begin();
    while(iter != events_.end()) {
        if((*iter)->getId() == id) {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        ++iter;
    }
}

void EventContainer::deleteType(int type)
{
    auto iter = events_.begin();
    while(iter != events_.end()) {
        if((*iter)->getType() == type) {
            delete (*iter);
            iter = events_.erase(iter);
            continue;
        }
        ++iter;
    }
}

bool EventContainer::empty() const
{
    return events_.empty();
}

Event* EventContainer::operator [](int id) const
{
    return events_[id];
}

void EventContainer::append(Event* event)
{
    events_.append(event);
}

void EventContainer::prepend(Event* event)
{
    events_.prepend(event);
}

void EventContainer::insertSorted(Event* event)
{
    auto iter = std::lower_bound(events_.begin(), events_.end(), event,
                        [](const Event* l, const Event* r) {
                            return l->getPriority() > r->getPriority();
                        });
    events_.insert(iter, event);
}

void EventContainer::insert(Event* event)
{
    if(event->isReady())
        delete event;
    else
        insertSorted(event);
}

typename EventContainer::Iterator EventContainer::begin()
{
    return events_.begin();
}

typename EventContainer::Iterator EventContainer::end()
{
    return events_.end();
}

typename EventContainer::ConstIterator EventContainer::begin() const
{
    return events_.begin();
}

typename EventContainer::ConstIterator EventContainer::end() const
{
    return events_.end();
}

void EventContainer::applyEvents(cv::Mat &frame) const
{
    for(auto iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->applyFrame(frame);
}

void EventContainer::pauseEvents()
{
    for(auto iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->pause();
}

void EventContainer::unpauseEvents()
{
    for(auto iter = events_.begin(); iter != events_.end(); ++iter)
        (*iter)->unpause();
}

int EventContainer::getTotalDuration() const
{
    return std::accumulate(begin(), end(), 0,
                           [](int x, const Event* a) {
                                return x + a->getStart() + a->getDelay();
                            });
}
