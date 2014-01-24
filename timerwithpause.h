#ifndef TIMER_H
#define TIMER_H

#include <QElapsedTimer>


/* Timer with pause function*/

class TimerWithPause : public QElapsedTimer
{

public:
    explicit TimerWithPause();

    void pause();
    void resume();
    void restart();

    qint64 nsecsElapsed() const;

private:
    qint64  time;
    bool    paused;
};

#endif // TIMER_H
