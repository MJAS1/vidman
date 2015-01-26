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
    //Multiple threads might request time
    QMutexLocker locker(&mutex_);
    if(paused_)
        return time_;
    else
        return (QElapsedTimer::nsecsElapsed() + time_);
}

void TimerWithPause::restart()
{
    time_ = 0;
    paused_ = false;
    QElapsedTimer::restart();
}
