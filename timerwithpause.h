#ifndef TIMER_H
#define TIMER_H

#include <QElapsedTimer>
#include <QTimer>

class TimerWithPause : public QObject, public QElapsedTimer
{
    Q_OBJECT
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
