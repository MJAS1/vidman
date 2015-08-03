#ifndef TIMER_H
#define TIMER_H

#include <QElapsedTimer>
#include <QMutex>

/* Timer with pause function*/

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
