#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw, QMutex& mutex, const OutputDevice& trigPort, LogFile& logFile,
                   QGLShaderProgram& sProg, QVector<QVector2D>& v1, QVector<QVector2D>& v2) :
    shouldSwap(false), isPaused(false), glw(glw), mutex(mutex), trigPort(trigPort),
    logFile(logFile), shaderProgram(sProg), vertices(v1), textureCoordinates(v2)
{
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {
        if(shouldSwap && !isPaused)
        {
            mutex.lock();

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
            mutex.unlock();
        }
    }
}

void GLThread::drawFrame(unsigned char* buf, int code, const QString& s)
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
