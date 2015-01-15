#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw, QMutex& mutex, LogFile& logFile) :
    shouldSwap(false), isPaused(false), glw(glw), mutex(mutex), logFile(logFile)
{
    vertices << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    mutex.lock();
    glw->makeCurrent();
    shaderProgram.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram.addShaderFromSourceFile(QGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram.link();
    glw->doneCurrent();
    mutex.unlock();
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {
        mutex.lock();
        if(shouldSwap && !isPaused)
        {
            glw->makeCurrent();

            shaderProgram.bind();
            shaderProgram.setUniformValue("texture", 0);
            shaderProgram.setAttributeArray("vertex", vertices.constData());
            shaderProgram.enableAttributeArray("vertex");
            shaderProgram.setAttributeArray("textureCoordinate", textureCoordinates.constData());
            shaderProgram.enableAttributeArray("textureCoordinate");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf);
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
            shouldSwap = false;
        }
        mutex.unlock();
    }
}

void GLThread::drawFrame(unsigned char* buf, int code, const QString& s)
{
    mutex.lock();

    imBuf = buf;
    trigCode = code;
    log = s;
    shouldSwap = true;

    mutex.unlock();
}

void GLThread::pause()
{
    mutex.lock();
    isPaused = true;
    mutex.unlock();
}

void GLThread::unpause()
{
    mutex.lock();
    isPaused = false;
    mutex.unlock();
}
bool GLThread::setOutputDevice(OutputDevice::PortType portType)
{
    mutex.lock();

    if(portType)
    {
        bool ok = trigPort.open(portType);
        mutex.unlock();
        return ok;
    }
    trigPort.close();

    mutex.unlock();

    return true;
}
