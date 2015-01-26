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
    explicit GLThread(GLVideoWidget *parent);

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
    void                stoppableRun();

    bool                shouldSwap_;
    bool                shouldChangePort_;
    bool                isPaused_;

    GLVideoWidget*	    glw_;

    QMutex               mutex_;
    LogFile& 			logFile_;
    unsigned char* 		imBuf_;

    QString 			    log_;
    OutputDevice 		trigPort_;
    OutputDevice::PortType newPort_;

    int trigCode_;

    QGLShaderProgram 	shaderProgram_;
    QVector<QVector2D> 	vertices_;
    QVector<QVector2D> 	textureCoordinates_;

};

#endif // GLTHREAD_H
