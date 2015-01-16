#include <QMutexLocker>
#include <QElapsedTimer>
#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw, QMutex& mutex, LogFile& logFile) :
    shouldSwap(false), isPaused(false), glw(glw), GLMutex(mutex), logFile(logFile)
{
    vertices << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    GLMutex.lock();
    glw->makeCurrent();
    shaderProgram.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram.addShaderFromSourceFile(QGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram.link();
    glw->doneCurrent();
    GLMutex.unlock();
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {

        //Local mutex should be unlocked even if shouldSwap is false, so use a mutex locker
        QMutexLocker locker(&localMutex);
        if(shouldSwap && !isPaused)
        {
            shouldSwap = false;

            GLMutex.lock();

            glw->makeCurrent();
            shaderProgram.bind();
            shaderProgram.setUniformValue("texture", 0);
            shaderProgram.setAttributeArray("vertex", vertices.constData());
            shaderProgram.enableAttributeArray("vertex");
            shaderProgram.setAttributeArray("textureCoordinate", textureCoordinates.constData());
            shaderProgram.enableAttributeArray("textureCoordinate");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf);

            //Unlock before drawing functions to prevent drawFrame() from stalling
            locker.unlock();

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glw->swapBuffers();

            shaderProgram.disableAttributeArray("vertex");
            shaderProgram.disableAttributeArray("textureCoordinate");
            shaderProgram.release();

            if(!trigPort.isEmpty())
                trigPort.writeData(trigCode);

            if(!log.isEmpty())
                logFile << log;

            glw->doneCurrent();

            GLMutex.unlock();

        }
    }
}

void GLThread::drawFrame(unsigned char* buf, int code, const QString& s)
{
    localMutex.lock();

    imBuf = buf;
    trigCode = code;
    log = s;
    shouldSwap = true;

    localMutex.unlock();
}

void GLThread::pause()
{
    localMutex.lock();
    isPaused = true;
    localMutex.unlock();
}

void GLThread::unpause()
{
    localMutex.lock();
    isPaused = false;
    localMutex.unlock();
}

bool GLThread::setOutputDevice(OutputDevice::PortType portType)
{
    QMutexLocker locker(&GLMutex);

    if(portType)
    {
        bool ok = trigPort.open(portType);
        return ok;
    }

    trigPort.close();
    return true;
}
