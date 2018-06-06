/*
 * eventcontainer.cpp
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <numeric>
#include <algorithm>

#include "eventcontainer.h"

using std::move;

EventContainer::EventContainer(QObject *parent) : QObject(parent)
{
}

void EventContainer::clear()
{
    events_.clear();
}

EventPtr EventContainer::pop_front()
{
    EventPtr ev = move(*events_.begin());
    events_.erase(events_.begin());
    return ev;
}

void EventContainer::deleteId(int id)
{
    auto end = std::remove_if(events_.begin(), events_.end(),
                        [&](const EventPtr& ev){return ev->getId() == id;});
    events_.erase(end, events_.end());
}

void EventContainer::deleteType(Event::EventType type)
{
    auto end = std::remove_if(events_.begin(), events_.end(),
                        [&](const EventPtr& ev){return ev->getType() == type;});
    events_.erase(end, events_.end());
}

bool EventContainer::empty() const
{
    return events_.empty();
}

const EventPtr& EventContainer::operator [](int id) const
{
    return events_[id];
}

void EventContainer::append(EventPtr event)
{
    events_.push_back(move(event));
}

void EventContainer::insertSorted(EventPtr event)
{
    // An event may have an effect on other events of this container
    event->apply(*this);

    if(!event->isReady()) {
        auto iter = std::lower_bound(events_.begin(), events_.end(), event,
                                     [](const EventPtr& l, const EventPtr& r) {
            return l->getPriority() > r->getPriority();}
        );
        events_.insert(iter, move(event));
    }
}

void EventContainer::applyEvents(cv::Mat &frame)
{
    for(auto iter = events_.begin(); iter != events_.end();)
    {
        (*iter)->apply(frame);
        if((*iter)->isReady())
            iter = events_.erase(iter);
        else
            iter++;
    }
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
    return std::accumulate(events_.begin(), events_.end(), 0,
                           [](int x, const EventPtr& a) {
        return x + a->getStart() + a->getDelay();
    });
}

void EventContainer::sort()
{
    std::sort(events_.begin(), events_.end(), [](const EventPtr& l,
              const EventPtr& r) {
        return l->getPriority() > r->getPriority();
    });
}
