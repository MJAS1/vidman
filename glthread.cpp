#include "glthread.h"
#include "glvideowidget.h"

GLThread::GLThread(GLVideoWidget *glw, QMutex *mutex) : shouldSwap(false), isPaused(false), glw(glw), mutex(mutex)
{
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {
        if(shouldSwap && !isPaused)
        {
            mutex->lock();

            glw->makeCurrent();
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glw->swapBuffers();
            glw->doneCurrent();

            shouldSwap = false;

            mutex->unlock();
        }
    }
}

void GLThread::swapBuffers()
{
    shouldSwap = true;
}

void GLThread::pause()
{
    isPaused = true;
}

void GLThread::unpause()
{
    isPaused = false;
}
