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


GLVideoWidget::GLVideoWidget(const QGLFormat& format, LogFile& logFile, VideoDialog* parent)
    : QGLWidget(format, parent), frames(0), videoWidth(VIDEO_WIDTH),
       glt(this, mutex, logFile)

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

    connect(&fpsTimer, SIGNAL(timeout()), this, SLOT(countFPS()));
    fpsTimer.start(1000);


}


GLVideoWidget::~GLVideoWidget()
{
    glt.stop();
    free(imBuf);
}


void GLVideoWidget::initializeGL()
{
    mutex.lock();

    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    mutex.unlock();

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

    mutex.lock();
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
    mutex.unlock();
}


void GLVideoWidget::onDrawFrame(unsigned char* imBuf, int logSize)
{

    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromLatin1((char*)(imBuf - logSize-1));

    doneCurrent();

    glt.drawFrame(imBuf, chunkAttrib.trigCode, log);
    frames++;

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
    Q_UNUSED(e)
    glt.unpause();
}

bool GLVideoWidget::setOutputDevice(OutputDevice::PortType portType)
{
    return glt.setOutputDevice(portType);
}
