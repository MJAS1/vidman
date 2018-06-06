/*
 * timerwithpause.h
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

#ifndef TIMER_H
#define TIMER_H

#include <QElapsedTimer>
#include <QMutex>

/*!
 * A simple thread-safe timer with pause functionality.
 */

class TimerWithPause : public QElapsedTimer
{

public:
    explicit TimerWithPause();

    void pause();
    void resume();
    void restart();

    qint64 nsecsElapsed() const;
    qint64 msecsElapsed() const;

private:

    mutable QMutex  mutex_;
    qint64  time_;
    bool    paused_;
};

#endif // TIMER_H
