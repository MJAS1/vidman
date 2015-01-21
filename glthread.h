#ifndef GLTHREAD_H
#define GLTHREAD_H

#include <QMutex>
#include <QGLShaderProgram>
#include "stoppablethread.h"
#include "outputdevice.h"
#include "logfile.h"

class GLVideoWidget;

/* This thread is used to swap the front and back frame buffers and
 * do the drawing on GLVideoWidget.*/

class GLThread : public StoppableThread
{
public:
    explicit GLThread(GLVideoWidget *parentGLW, LogFile& logfile);

    void drawFrame(unsigned char* imBuf, int trigCode, const QString& log);

    /*The buffer swap loop needs to be paused when glw is being resized to prevent
     * a segmentation fault. */
    void pause();
    void unpause();

    /*Because ioperm sets port permissions only for the calling thread, it is important
    that trigPort.open() is called by this thread. This must be done in stoppableRun loop.
    This function is used notify that trigPort should be set to a given port.*/
    void setOutputDevice(OutputDevice::PortType portType);

private:
    void stoppableRun();

    bool shouldSwap;
    bool shouldChangePort;
    bool isPaused;

    GLVideoWidget*		glw;

    QMutex              mutex;
    LogFile& 			logFile;
    unsigned char* 		imBuf;

    QString 			log;
    OutputDevice 		trigPort;
    OutputDevice::PortType newPort;

    int trigCode;

    QGLShaderProgram 	shaderProgram;
    QVector<QVector2D> 	vertices;
    QVector<QVector2D> 	textureCoordinates;

};

#endif // GLTHREAD_H
