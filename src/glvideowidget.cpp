/*
 * glvideowidget.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <QResizeEvent>
#include <QWindow>
#include "mainwindow.h"
#include "videodialog.h"
#include "glvideowidget.h"
#include "cycdatabuffer.h"
#include "common.h"

using namespace std;

GLVideoWidget::GLVideoWidget(const QGLFormat& format, VideoDialog* parent)
    : QGLWidget(format, parent), frames_(0), videoWidth_(VIDEO_WIDTH),
       videoDialog_(parent)
{
    //Important to manually swap the framebuffers for syncing trigger signals.
    setAutoBufferSwap(false);

    connect(&fpsTimer_, SIGNAL(timeout()), this, SLOT(updateFPS()));

    connect(videoDialog_, SIGNAL(drawFrame(unsigned char*)), this,
            SLOT(onDrawFrame()));
    connect(this, SIGNAL(startScript()), videoDialog_->mainWindow(),
            SLOT(onStartButton()));
    connect(this, SIGNAL(increaseAspectRatio()), videoDialog_,
            SLOT(increaseAspectRatio()));
    connect(this, SIGNAL(decreaseAspectRatio()), videoDialog_,
            SLOT(decreaseAspectRatio()));

    fpsTimer_.start(1000);
}

GLVideoWidget::~GLVideoWidget()
{
}

void GLVideoWidget::paintEvent(QPaintEvent *)
{
    /*
     * Reimplement an empty paintEvent function. This function is automatically
     * called by the main thread everytime the window is resized. The default
     * implementation would in turn call makeCurrent() which will cause a
     * warning "QGLContext::makeCurrent(): Failed." because the context is
     * active in GLThread where all the drawing should be done. This
     * reimplementation should therefore remove the warnings.
     */
}

void GLVideoWidget::onDrawFrame()
{
    frames_++;
}

void GLVideoWidget::updateFPS()
{
    videoDialog_->updateFPS(frames_);
    frames_ = 0;
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    // TODO: Fix black screen issue after exiting full screen. This problem was
    // introduced after upgrading to Qt5 from Qt4.8.
    if(isFullScreen()) {
        setWindowFlags(Qt::Widget);
        showNormal();
		videoDialog_->show();
    }
    else {
        setWindowFlags(Qt::Window);
		videoDialog_->close();
        showFullScreen();
    }
}

void GLVideoWidget::resizeEvent(QResizeEvent *e)
{
    emit resize(e->size().width(), e->size().height());
}

VideoDialog* GLVideoWidget::videoDialog()
{
    return videoDialog_;
}

void GLVideoWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Space)
        emit startScript();
    else if(ev->key() == Qt::Key_Plus)
        emit increaseAspectRatio();
    else if(ev->key() == Qt::Key_Minus)
        emit decreaseAspectRatio();
}
