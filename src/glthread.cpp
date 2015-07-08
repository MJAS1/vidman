#include <QMutexLocker>
#include <QElapsedTimer>
#include <videodialog.h>
#include "mainwindow.h"
#include "glthread.h"
#include "glvideowidget.h"
#include "iostream"

GLThread::GLThread(GLVideoWidget *glw) :
    StoppableThread(static_cast<QObject*>(glw)), shouldSwap_(false), shouldChangePort_(false),
    glw_(glw), videoWidth_(VIDEO_WIDTH)
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

    shaderProgram_.addShaderFromSourceFile(QGLShader::Vertex, ":/src/vertexShader.vsh");
    shaderProgram_.addShaderFromSourceFile(QGLShader::Fragment, ":/src/fragmentShader.fsh");
    shaderProgram_.link();
    glw_->doneCurrent();
}

void GLThread::stoppableRun()
{
    while(!shouldStop) {

        QMutexLocker locker(&mutex_);

        //Mutex has to be unlocked before trigCode_ and log_ are used and they might therefore get a
        //new value before use, so copy them to a local scope variable
        int trigCode = trigCode_;
        QString log = log_;

        //trigPort.open() must be called by this thread to make outb() work
        if(shouldChangePort_) {
            if(newPort_)
                trigPort_.open(newPort_);
            else
                trigPort_.close();

            shouldChangePort_ = false;
        }

        if(shouldSwap_) {
            shouldSwap_ = false;

            glMutex_.lock();
            glw_->makeCurrent();

            shaderProgram_.bind();
            shaderProgram_.setUniformValue("texture", 0);
            shaderProgram_.setAttributeArray("vertex", vertices_.constData());
            shaderProgram_.enableAttributeArray("vertex");
            shaderProgram_.setAttributeArray("textureCoordinate", textureCoordinates_.constData());
            shaderProgram_.enableAttributeArray("textureCoordinate");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf_);

            //Unlock before drawing functions to prevent drawFrame() from stalling the main thread
            locker.unlock();

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glw_->swapBuffers();

            shaderProgram_.disableAttributeArray("vertex");
            shaderProgram_.disableAttributeArray("textureCoordinate");
            shaderProgram_.release();

            glw_->doneCurrent();
            glMutex_.unlock();

            if(!trigPort_.isEmpty())
                trigPort_.writeData(trigCode);

            if(!log.isEmpty())
                glw_->videoDialog()->mainWindow()->writeToLog(log);
        }
    }
}

void GLThread::drawFrame(unsigned char* imBuf, ChunkAttrib& attrib)
{
    mutex_.lock();

    imBuf_ = imBuf;
    trigCode_ = attrib.trigCode;
    log_ = QString::fromLatin1(attrib.log);
    shouldSwap_ = true;

    mutex_.unlock();
}

void GLThread::setOutputDevice(OutputDevice::PortType newPort)
{
    mutex_.lock();
    newPort_ = newPort;
    shouldChangePort_ = true;
    mutex_.unlock();
}

void GLThread::resizeGL(int w, int h)
{
    int dispW;
    int dispH;

    // Change the viewport to preserve the aspect ratio.
    // Compute new height corresponding to the current width and new width
    // corresponding to the current heigh and see which one fits.
    dispH = int(floor((w / float(videoWidth_)) * VIDEO_HEIGHT));
    dispW = int(floor((h / float(VIDEO_HEIGHT)) * videoWidth_));

    /*This function is called by the main GUI thread so the OpenGL context has to
     * be made current in it with the makeCurrent() call. Call doneCurrent() after
     * finished. The mutex is used to sync the makeCurrent() calls between the
     * threads.*/
    glMutex_.lock();
    glw_->makeCurrent();

    if(dispH <= h)
        glViewport(0, (h - dispH) / 2, w, dispH);

    else if(dispW <= w)
        glViewport((w - dispW) / 2, 0, dispW, h);

    else {
        std::cerr << "Internal error while computing the viewport size" << std::endl;
        abort();
    }

    glw_->doneCurrent();
    glMutex_.unlock();
}

void GLThread::setVideoWidth(int width)
{
    videoWidth_ = width;
    resizeGL(glw_->width(), glw_->height());
}
