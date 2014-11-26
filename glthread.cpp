#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glw->swapBuffers();
            glw->doneCurrent();

            shouldSwap = false;

            mutex->unlock();
        }
    }
}

void GLThread::swapBuffers(unsigned char* buf)
{
    imBuf = buf;
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
