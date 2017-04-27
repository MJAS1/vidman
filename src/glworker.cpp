#include <iostream>
#include <cstring>
#include <QCoreApplication>
#include <QWindow>
#include "videodialog.h"
#include "mainwindow.h"
#include "common.h"
#include "glworker.h"
#include "glvideowidget.h"

GLWorker::GLWorker(GLVideoWidget *glw) :
    glw_(glw), videoWidth_(VIDEO_WIDTH), shouldStop_(false)
{
    buf_ = nullptr;
    vertices_ << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates_ << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    initializeGL();
}

void GLWorker::initializeGL()
{
    glw_->makeCurrent();
    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    shaderProgram_.addShaderFromSourceFile(QGLShader::Vertex,
                                           ":/src/vertexShader.vsh");
    shaderProgram_.addShaderFromSourceFile(QGLShader::Fragment,
                                           ":/src/fragmentShader.fsh");
    shaderProgram_.link();
    glw_->doneCurrent();
}

/* start() and stop() are called by the main thread. However, the loop is
 * supposed to be working in a different thread. invokeMethod() makes sure that
 * the call for startLoop() and stopLoop is placed in the thread's event queue.
 */
void GLWorker::start()
{
    QMetaObject::invokeMethod(this, "startLoop", Qt::QueuedConnection);
}

void GLWorker::stop()
{
    QMetaObject::invokeMethod(this, "stopLoop", Qt::BlockingQueuedConnection);
}

void GLWorker::onDrawFrame(unsigned char *imBuf)
{
    buf_ = imBuf;
}

void GLWorker::resizeGL(int w, int h)
{
    /* Change the viewport to preserve the aspect ratio. Compute new height
     * corresponding to the current width and new width corresponding to the
     * current heigh and see which one fits.
     */
    int dispW = int(floor((h / float(VIDEO_HEIGHT)) * videoWidth_));;
    int dispH = int(floor((w / float(videoWidth_)) * VIDEO_HEIGHT));;

    if(dispH <= h)
        glViewport(0, (h - dispH) / 2, w, dispH);
    else if(dispW <= w)
        glViewport((w - dispW) / 2, 0, dispW, h);
    else {
        std::cerr << "Internal error while computing the viewport size"
                  << std::endl;
        abort();
    }
}

void GLWorker::onAspectRatioChanged(int w)
{
    videoWidth_ = w;
    resizeGL(glw_->width(), glw_->height());
}

void GLWorker::startLoop()
{
    glw_->makeCurrent();

    /*Wait until the first frame is ready and window is exposed to avoid
     * warnings. */

    while(!(buf_ && glw_->windowHandle()->isExposed())) {
        emit vblank();
        QCoreApplication::processEvents();
    }

    shaderProgram_.bind();
    shaderProgram_.setUniformValue("texture", 0);
    shaderProgram_.setAttributeArray("vertex", vertices_.constData());
    shaderProgram_.enableAttributeArray("vertex");
    shaderProgram_.setAttributeArray("textureCoordinate",
                                     textureCoordinates_.constData());
    shaderProgram_.enableAttributeArray("textureCoordinate");

    while(!shouldStop_)
    {
        //Start drawing the frames.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)buf_);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glw_->swapBuffers();

        /* With vsync enabled glFinish() should stop the thread until a v-blank
         * signal has been received, meaning that the screen image has been
         * updated with the front framebuffer. The execution of the code
         * following glFinish() should then sync with the screen refresh rate.
         */
        glFinish();
        emit vblank();

        ChunkAttrib chunkAttrib = *((ChunkAttrib*)(buf_-sizeof(ChunkAttrib)));
        emit triggerSignal(chunkAttrib.trigCode);
        if(strlen(chunkAttrib.log))
            emit log(QString(chunkAttrib.log));

        QCoreApplication::processEvents();
    }

    shaderProgram_.disableAttributeArray("vertex");
    shaderProgram_.disableAttributeArray("textureCoordinate");
    shaderProgram_.release();
}

void GLWorker::stopLoop()
{
    shouldStop_ = true;
}
