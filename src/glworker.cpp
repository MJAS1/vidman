#include <iostream>
#include <cstring>
#include <QDebug>
#include "videodialog.h"
#include "mainwindow.h"
#include "common.h"
#include "glworker.h"
#include "glvideowidget.h"

GLWorker::GLWorker(GLVideoWidget *glw) : QObject(), glw_(glw),
    videoWidth_(VIDEO_WIDTH)
{
    qRegisterMetaType<OutputDevice::PortType>("OutputDevice::PortType");
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

    shaderProgram_.addShaderFromSourceFile(QGLShader::Vertex, ":/src/vertexShader.vsh");
    shaderProgram_.addShaderFromSourceFile(QGLShader::Fragment, ":/src/fragmentShader.fsh");
    shaderProgram_.link();
    glw_->doneCurrent();
}

void GLWorker::onDrawFrame(unsigned char *imBuf)
{
    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)));

    glw_->makeCurrent();

    shaderProgram_.bind();
    shaderProgram_.setUniformValue("texture", 0);
    shaderProgram_.setAttributeArray("vertex", vertices_.constData());
    shaderProgram_.enableAttributeArray("vertex");
    shaderProgram_.setAttributeArray("textureCoordinate", textureCoordinates_.constData());
    shaderProgram_.enableAttributeArray("textureCoordinate");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, VIDEO_WIDTH, VIDEO_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)imBuf);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glw_->swapBuffers();

    shaderProgram_.disableAttributeArray("vertex");
    shaderProgram_.disableAttributeArray("textureCoordinate");
    shaderProgram_.release();

    if(!trigPort_.isEmpty())
        trigPort_.writeData(chunkAttrib.trigCode);

    if(strlen(chunkAttrib.log))
        glw_->videoDialog()->mainWindow()->writeToLog(QString(chunkAttrib.log));
}

void GLWorker::resizeGL(int w, int h)
{
    int dispW;
    int dispH;

    // Change the viewport to preserve the aspect ratio.
    // Compute new height corresponding to the current width and new width
    // corresponding to the current heigh and see which one fits.
    dispH = int(floor((w / float(videoWidth_)) * VIDEO_HEIGHT));
    dispW = int(floor((h / float(VIDEO_HEIGHT)) * videoWidth_));

    glw_->makeCurrent();

    if(dispH <= h)
        glViewport(0, (h - dispH) / 2, w, dispH);

    else if(dispW <= w)
        glViewport((w - dispW) / 2, 0, dispW, h);

    else {
        std::cerr << "Internal error while computing the viewport size" << std::endl;
        abort();
    }
}

void GLWorker::onAspectRatioChanged(int w)
{
    videoWidth_ = w;
    resizeGL(glw_->width(), glw_->height());
}

void GLWorker::setOutputDevice(OutputDevice::PortType portType)
{
    if(portType)
        trigPort_.open(portType);
    else
        trigPort_.close();
}
