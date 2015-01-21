#include <QMutexLocker>
#include <QElapsedTimer>
#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw, LogFile& logFile) :
    shouldSwap(false), shouldChangePort(false), isPaused(false), glw(glw), logFile(logFile)
{
    vertices << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    glw->makeCurrent();
    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    shaderProgram.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram.addShaderFromSourceFile(QGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram.link();
    glw->doneCurrent();
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {

        QMutexLocker locker(&mutex);

        //trigPort.open() must be called in this thread to make outb work
        if(shouldChangePort)
        {
            if(newPort)
            {
                trigPort.open(newPort);
            }
            else
                trigPort.close();

            shouldChangePort = false;
        }

        if(shouldSwap && !isPaused)
        {
            shouldSwap = false;

            glw->makeCurrent();
            shaderProgram.bind();
            shaderProgram.setUniformValue("texture", 0);
            shaderProgram.setAttributeArray("vertex", vertices.constData());
            shaderProgram.enableAttributeArray("vertex");
            shaderProgram.setAttributeArray("textureCoordinate", textureCoordinates.constData());
            shaderProgram.enableAttributeArray("textureCoordinate");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf);

            //Unlock before drawing functions to prevent drawFrame() from stalling
            mutex.unlock();

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
        }
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

void GLThread::setOutputDevice(OutputDevice::PortType portType)
{
    mutex.lock();
    newPort = portType;
    shouldChangePort = true;
    mutex.unlock();
}
