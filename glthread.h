#ifndef GLTHREAD_H
#define GLTHREAD_H

#include <QMutex>
#include "stoppablethread.h"

class GLVideoWidget;

/* This thread is used to swap the front and back frame buffers and
 * do the drawing on GLVideoWidget.*/

class GLThread : public StoppableThread
{
public:
    GLThread(GLVideoWidget *glw, QMutex *mutex);

    void swapBuffers();

    /*The buffer swap loop needs to be paused when glw is being resized to prevent
     * a segmentation fault. */
    void pause();
    void unpause();

private:
    void stoppableRun();

    bool shouldSwap, isPaused;

    GLVideoWidget *glw;
    QMutex *mutex;
};

#endif // GLTHREAD_H
