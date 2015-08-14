/*
 * glvideowidget.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <QResizeEvent>
#include "videodialog.h"
#include "glvideowidget.h"
#include "cycdatabuffer.h"
#include "common.h"

using namespace std;

GLVideoWidget::GLVideoWidget(const QGLFormat& format, VideoDialog* parent)
    : QGLWidget(format, parent), frames_(0), videoWidth_(VIDEO_WIDTH),
       videoDialog_(parent), glworker_(this)
{
    setAutoBufferSwap(false);

    connect(&fpsTimer_, SIGNAL(timeout()), this, SLOT(updateFPS()));
    connect(videoDialog_, SIGNAL(drawFrame(unsigned char*)), &glworker_, SLOT(onDrawFrame(unsigned char*)));
    connect(videoDialog_, SIGNAL(drawFrame(unsigned char*)), this, SLOT(onDrawFrame()));
    connect(videoDialog_, SIGNAL(aspectRatioChanged(int)), &glworker_, SLOT(onAspectRatioChanged(int)));  
    qRegisterMetaType<OutputDevice::PortType>("OutputDevice::PortType");
    connect(videoDialog_, SIGNAL(outputDeviceChanged(OutputDevice::PortType)), &glworker_, SLOT(setOutputDevice(OutputDevice::PortType)));
    connect(this, SIGNAL(resize(int,int)), &glworker_, SLOT(resizeGL(int,int)));

    fpsTimer_.start(1000);

    /*QGLContext::moveToThread() was introduced in Qt5 and is necessary to
     * enable OpenGL in a different thread.
     * */
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    context()->moveToThread(&glthread_);
#endif
    glworker_.moveToThread(&glthread_);
    glthread_.start();
}

GLVideoWidget::~GLVideoWidget()
{
    glthread_.quit();
    glthread_.wait();
}

void GLVideoWidget::paintEvent(QPaintEvent *)
{
    /*Reimplement an empty paintEvent function. This function is automatically
    *called by the main thread everytime the window is resized. The default
    *implementation would in turn call makeCurrent() which will cause a warning
    *"QGLContext::makeCurrent(): Failed." because the context is active in
    *GLThread where all the drawing should be done. This reimplementation should
    *therefore remove the warnings.*/
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
    if(isFullScreen()) {
        setWindowFlags(Qt::Widget);
		videoDialog_->show();
        showNormal();
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
