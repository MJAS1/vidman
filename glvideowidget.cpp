/*
 * glvideowidget.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <iostream>
#include <math.h>
#include <QElapsedTimer>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <QMessageBox>
#include <QResizeEvent>
#include <unistd.h>
#include "videodialog.h"
#include "config.h"
#include "glvideowidget.h"
#include "cycdatabuffer.h"

using namespace std;


GLVideoWidget::GLVideoWidget(const QGLFormat& format, OutputDevice *trigPort, LogFile& logFile, VideoDialog* parent)
    : QGLWidget(format, parent), fpsTimer(new QTimer(this)), frames(0), videoWidth(VIDEO_WIDTH), trigPort(trigPort),
       glt(this, &mutex, trigPort, logFile)

{
    setAutoBufferSwap(false);

    // Allocate memory. Since we do not know whether the image is going to BW
    // or color, allocate for color since it requires more memory.
    imBuf = (char*)malloc(VIDEO_HEIGHT*VIDEO_WIDTH*3);
    if (!imBuf)
    {
        cerr << "Error allocating memory" << endl;
        exit(EXIT_FAILURE);
    }

    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(countFPS()));
    fpsTimer->start(1000);


}


GLVideoWidget::~GLVideoWidget()
{
    glt.stop();
    free(imBuf);
}


void GLVideoWidget::initializeGL()
{
    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    shaderProgram.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram.addShaderFromSourceFile(QGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram.link();

    vertices << QVector2D(-1, 1) << QVector2D(-1, -1) << QVector2D(1, -1)
             << QVector2D(1, -1) << QVector2D(1, 1) << QVector2D(-1, 1);

    textureCoordinates << QVector2D(0, 0) << QVector2D(0, 1) << QVector2D(1, 1)
                       << QVector2D(1, 1) << QVector2D(1, 0) << QVector2D(0, 0);

    glt.start();

}

void GLVideoWidget::paintGL()
{
}

void GLVideoWidget::resizeGL(int _w, int _h)
{
    int dispW;
    int dispH;

    // Change the viewport to preserve the aspect ratio.
    // Compute new height corresponding to the current width and new width
    // corresponding to the current heigh and see which one fits.
    dispH = int(floor((_w / float(videoWidth)) * VIDEO_HEIGHT));
    dispW = int(floor((_h / float(VIDEO_HEIGHT)) * videoWidth));

    makeCurrent();

    if(dispH <= _h)
    {
        glViewport(0, (_h - dispH) / 2, _w, dispH);
    }
    else if(dispW <= _w)
    {
        glViewport((_w - dispW) / 2, 0, dispW, _h);
    }
    else
    {
        cerr << "Internal error while computing the viewport size" << endl;
        abort();
    }

    doneCurrent();
}


void GLVideoWidget::onDrawFrame(unsigned char* imBuf, int logSize)
{
    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromLatin1((char*)(imBuf - logSize-1));

    //makeCurrent();

    shaderProgram.bind();

    shaderProgram.setUniformValue("texture", 0);

    shaderProgram.setAttributeArray("vertex", vertices.constData());
    shaderProgram.enableAttributeArray("vertex");

    shaderProgram.setAttributeArray("textureCoordinate", textureCoordinates.constData());
    shaderProgram.enableAttributeArray("textureCoordinate");

    doneCurrent();

    glt.swapBuffers(imBuf, chunkAttrib.trigCode, log);

    shaderProgram.disableAttributeArray("vertex");
    shaderProgram.disableAttributeArray("textureCoordinate");

    shaderProgram.release();


    frames++;
    //Write to log file. Parent widget must be a VideoDialog.
    //if(logSize)
     //   static_cast<VideoDialog*>(parent())->writeToLogFile(log);

}

void GLVideoWidget::countFPS()
{
    int fps = frames;
    frames = 0;
    static_cast<VideoDialog*>(parent())->setFPS(fps);
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    glt.pause();

    if(isFullScreen())
    {
        setWindowFlags(Qt::Widget);
        showNormal();

    }
    else
    {
        setWindowFlags(Qt::Window);
        showFullScreen();
    }
}

void GLVideoWidget::setVideoWidth(int newVal)
{
    videoWidth = newVal;
    resizeGL(width(), height());
}

void GLVideoWidget::resizeEvent(QResizeEvent *e)
{
    if(e->oldSize().width() > 100)
        glt.pause();

    resizeGL(e->size().width(), e->size().height());
}

void GLVideoWidget::mousePressEvent(QMouseEvent *e)
{
    glt.unpause();
}
