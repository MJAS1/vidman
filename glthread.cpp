#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw, QMutex *mutex, OutputDevice* trigPort, LogFile& logFile) : shouldSwap(false), isPaused(false), glw(glw),
    mutex(mutex), trigPort(trigPort), logFile(logFile)
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

            if(!trigPort->isEmpty())
                trigPort->writeData(trigCode);

            if(!log.isEmpty())
                logFile << log;

            glw->doneCurrent();

            shouldSwap = false;

            mutex->unlock();
        }
    }
}

void GLThread::swapBuffers(unsigned char* buf, int code, const QString& s)
{
    imBuf = buf;
    trigCode = code;
    log = s;
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
