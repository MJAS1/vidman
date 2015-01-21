#include <QMutexLocker>
#include "timerwithpause.h"

TimerWithPause::TimerWithPause() :
    QElapsedTimer(), time(0), paused(false)
{
}

void TimerWithPause::pause()
{
    time = nsecsElapsed();
    paused = true;
}

void TimerWithPause::resume()
{
    QElapsedTimer::restart();
    paused = false;
}

qint64 TimerWithPause::nsecsElapsed()
{
    //Multiple threads might request time
    QMutexLocker locker(&mutex);
    if(paused)
        return time;
    else
        return (QElapsedTimer::nsecsElapsed() + time);
}

void TimerWithPause::restart()
{
    time = 0;
    paused = false;
    QElapsedTimer::restart();
}
