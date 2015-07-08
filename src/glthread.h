#ifndef GLTHREAD_H
#define GLTHREAD_H

#include <QMutex>
#include <QGLShaderProgram>
#include "stoppablethread.h"
#include "cycdatabuffer.h"
#include "outputdevice.h"
#include "logfile.h"

class GLVideoWidget;

/* This thread is used to swap the front and back frame buffers and
 * do the drawing on GLVideoWidget.*/

class GLThread : public StoppableThread
{
public:
    explicit GLThread(GLVideoWidget *parent);

    void drawFrame(unsigned char* imBuf, ChunkAttrib& attrib);
    void resizeGL(int w, int h);
    void setVideoWidth(int width);

    /*Because ioperm sets port permissions only for the calling thread, it is important
    that trigPort.open() is called by this thread. This must be done in stoppableRun loop.
    This function is used to notify that trigPort should be set to a given port.*/
    void setOutputDevice(OutputDevice::PortType portType);

private:
    void                    stoppableRun();

    bool                    shouldSwap_;
    bool                    shouldChangePort_;

    GLVideoWidget*          glw_;

    QMutex                  mutex_;
    QMutex                  glMutex_;

    unsigned char*          imBuf_;

    QString 			    log_;
    OutputDevice            trigPort_;
    OutputDevice::PortType  newPort_;

    int trigCode_;
    int videoWidth_;

    QGLShaderProgram        shaderProgram_;
    QVector<QVector2D>      vertices_;
    QVector<QVector2D>      textureCoordinates_;
};

#endif // GLTHREAD_H
