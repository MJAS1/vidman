#include <QMutexLocker>
#include "timerwithpause.h"

TimerWithPause::TimerWithPause() :
    QElapsedTimer(), time_(0), paused_(false)
{
}

void TimerWithPause::pause()
{
    time_ = nsecsElapsed();
    paused_ = true;
}

void TimerWithPause::resume()
{
    QElapsedTimer::restart();
    paused_ = false;
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
        return time_;
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
