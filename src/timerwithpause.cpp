/*
 * timerwithpause.cpp
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

#include <QMutexLocker>

#include "timerwithpause.h"

TimerWithPause::TimerWithPause() :
    time_(0), paused_(false)
{
}

void TimerWithPause::pause()
{
    time_ = nsecsElapsed();
    mutex_.lock();
    paused_ = true;
    mutex_.unlock();
}

void TimerWithPause::resume()
{
    mutex_.lock();
    QElapsedTimer::restart();
    paused_ = false;
    mutex_.unlock();
}

qint64 TimerWithPause::nsecsElapsed() const
{
    QMutexLocker locker(&mutex_);
    if(paused_)
        return time_;
    else
        return (QElapsedTimer::nsecsElapsed() + time_);
}

qint64 TimerWithPause::msecsElapsed() const
{
    QMutexLocker locker(&mutex_);
    if(paused_)
        return time_/1000000;
    else
        return ((QElapsedTimer::nsecsElapsed() + time_)/1000000);
}

void TimerWithPause::restart()
{
    mutex_.lock();
    time_ = 0;
    paused_ = false;
    QElapsedTimer::restart();
    mutex_.unlock();
}
