#include <QMutexLocker>
#include <QElapsedTimer>
#include <videodialog.h>
#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw) :
    shouldSwap_(false), shouldChangePort_(false), isPaused_(false), glw_(glw),//, logFile_(logFile)
    logFile_(glw_->videoDialog()->logFile())
{
    vertices_ << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates_ << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    //Initialize openGL
    glw_->makeCurrent();
    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    shaderProgram_.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram_.addShaderFromSourceFile(QGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram_.link();
    glw_->doneCurrent();
}

void GLThread::stoppableRun()
{
    while(!shouldStop)
    {

        QMutexLocker locker(&mutex_);

        //Mutex has to be unlocked before trigCode_ is used and it might therefore get a
        //new value before use, so copy it to a local scope variable
        int trigCode = trigCode_;

        //trigPort.open() must be called in this thread to make outb work
        if(shouldChangePort_)
        {
            if(newPort_)
            {
                trigPort_.open(newPort_);
            }
            else
                trigPort_.close();

            shouldChangePort_ = false;
        }

        if(shouldSwap_ && !isPaused_)
        {
            shouldSwap_ = false;

            glw_->makeCurrent();
            shaderProgram_.bind();
            shaderProgram_.setUniformValue("texture", 0);
            shaderProgram_.setAttributeArray("vertex", vertices_.constData());
            shaderProgram_.enableAttributeArray("vertex");
            shaderProgram_.setAttributeArray("textureCoordinate", textureCoordinates_.constData());
            shaderProgram_.enableAttributeArray("textureCoordinate");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf_);

            //Unlock before drawing functions to prevent drawFrame() from stalling
            locker.unlock();

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glw_->swapBuffers();

            shaderProgram_.disableAttributeArray("vertex");
            shaderProgram_.disableAttributeArray("textureCoordinate");
            shaderProgram_.release();

            if(!trigPort_.isEmpty())
                trigPort_.writeData(trigCode);

            if(!log_.isEmpty())
                logFile_ << log_;

            glw_->doneCurrent();
        }
    }
}

void GLThread::drawFrame(unsigned char* imBuf, int trigCode, const QString& log)
{
    mutex_.lock();

    imBuf_ = imBuf;
    trigCode_ = trigCode;
    log_ = log;
    shouldSwap_ = true;

    mutex_.unlock();
}

void GLThread::pause()
{
    mutex_.lock();
    isPaused_ = true;
    mutex_.unlock();
}

void GLThread::unpause()
{
    mutex_.lock();
    isPaused_ = false;
    mutex_.unlock();
}

void GLThread::setOutputDevice(OutputDevice::PortType newPort)
{
    mutex_.lock();
    newPort_ = newPort;
    shouldChangePort_ = true;
    mutex_.unlock();
}
