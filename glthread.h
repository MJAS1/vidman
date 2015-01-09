#ifndef GLTHREAD_H
#define GLTHREAD_H

#include <QMutex>
#include "stoppablethread.h"
#include "outputdevice.h"
#include "logfile.h"

class GLVideoWidget;

/* This thread is used to swap the front and back frame buffers and
 * do the drawing on GLVideoWidget.*/

class GLThread : public StoppableThread
{
public:
    GLThread(GLVideoWidget *glw, QMutex *mutex, OutputDevice *trigPort, LogFile& logfile);

    void swapBuffers(unsigned char* imBuf, int trigCode, const QString& log);

    /*The buffer swap loop needs to be paused when glw is being resized to prevent
     * a segmentation fault. */
    void pause();
    void unpause();

private:
    void stoppableRun();

    bool shouldSwap, isPaused;

    GLVideoWidget *glw;
    QMutex *mutex;
    OutputDevice* trigPort;
    LogFile& logFile;
    unsigned char* imBuf;
    QString log;

    int trigCode;
};

#endif // GLTHREAD_H
