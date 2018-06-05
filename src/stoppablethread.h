#ifndef STOPPABLETHREAD_H_
#define STOPPABLETHREAD_H_

#include <QThread>

//! Extension of QThread with added simple mechanism for stopping the thread.
/*!
 * This class adds a simple mechanism for stopping the thread. The thread is
 * stopped by calling stop() method which sets shouldStop protected variable
 * to true. User should override stoppableRun() method to do the actual job
 * (similar to the run() method of Qthread). User implementation of
 * stoppableRun() should check shouldStop variable regularly and return from
 * stoppableRun() if it is true. The stoppableRun() method should only be
 * called once in the object's lifetime.
 *
 * This class prints logging messages to the standard output when a new
 * object is created/run/stopped/destroyed. The code for these messages is
 * not properly synchronized by semaphores, so under certain (probably quite
 * rare) race conditions these messages can be complete garbage. However,
 * this should not affect other functionality of the class.
 */

class StoppableThread : public QThread
{
public:
    explicit StoppableThread(QObject *parent = 0);
    virtual ~StoppableThread();

    void stop();

protected:
    virtual void run();
    virtual void stoppableRun() = 0;
    volatile bool shouldStop_;

private:
    static int nextId_;
    int id_;
};

#endif /* STOPPABLETHREAD_H_ */
