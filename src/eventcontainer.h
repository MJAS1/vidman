/*
 * eventcontainer.h
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

    // Insert a new event and sort according to priority.
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
