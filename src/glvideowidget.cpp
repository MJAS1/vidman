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


GLVideoWidget::GLVideoWidget(const QGLFormat& format, VideoDialog* parent)
    : QGLWidget(format, parent), frames_(0), videoWidth_(VIDEO_WIDTH),
       videoDialog_(parent), glt_(new GLThread(this))

{
    setAutoBufferSwap(false);

    connect(&fpsTimer_, SIGNAL(timeout()), this, SLOT(displayFPS()));
    fpsTimer_.start(1000);

    glt_->start();
}


GLVideoWidget::~GLVideoWidget()
{
    glt_->stop();
    delete glt_;
}


void GLVideoWidget::initializeGL()
{
}

void GLVideoWidget::paintGL()
{
}

void GLVideoWidget::resizeGL(int w, int h)
{
    int dispW;
    int dispH;

    // Change the viewport to preserve the aspect ratio.
    // Compute new height corresponding to the current width and new width
    // corresponding to the current heigh and see which one fits.
    dispH = int(floor((w / float(videoWidth_)) * VIDEO_HEIGHT));
    dispW = int(floor((h / float(VIDEO_HEIGHT)) * videoWidth_));

    makeCurrent();

    if(dispH <= h)
        glViewport(0, (h - dispH) / 2, w, dispH);

    else if(dispW <= w)
        glViewport((w - dispW) / 2, 0, dispW, h);

    else {
        cerr << "Internal error while computing the viewport size" << endl;
        abort();
    }

    doneCurrent();
}


void GLVideoWidget::onDrawFrame(unsigned char* imBuf, int logSize)
{

    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromLatin1((char*)(imBuf - logSize-1));

    doneCurrent();
    glt_->drawFrame(imBuf, chunkAttrib.trigCode, log);
    frames_++;

}

void GLVideoWidget::displayFPS()
{
    videoDialog_->displayFPS(frames_);
    frames_ = 0;
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    glt_->pause();

    if(isFullScreen()) {
        setWindowFlags(Qt::Widget);
        showNormal();
    }
    else {
        setWindowFlags(Qt::Window);
        showFullScreen();
    }

}

void GLVideoWidget::setVideoWidth(int newVal)
{
    videoWidth_ = newVal;
    glt_->pause();
    resizeGL(width(), height());
}

void GLVideoWidget::resizeEvent(QResizeEvent *e)
{

    if(e->oldSize().width() > 100)
        glt_->pause();

    resizeGL(e->size().width(), e->size().height());
    glt_->unpause();
}

void GLVideoWidget::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    glt_->unpause();
}

void GLVideoWidget::setOutputDevice(OutputDevice::PortType portType)
{
    glt_->setOutputDevice(portType);
}

VideoDialog* GLVideoWidget::videoDialog()
{
    return videoDialog_;
}
