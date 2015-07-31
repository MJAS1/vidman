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
#include "glthread.h"

using namespace std;


GLVideoWidget::GLVideoWidget(const QGLFormat& format, VideoDialog* parent)
    : QGLWidget(format, parent), frames_(0), videoWidth_(VIDEO_WIDTH),
       videoDialog_(parent), glt_(new GLThread(this))
{
    setAutoBufferSwap(false);

    connect(&fpsTimer_, SIGNAL(timeout()), this, SLOT(updateFPS()));
    fpsTimer_.start(1000);

    glt_->start();
}

GLVideoWidget::~GLVideoWidget()
{
    glt_->stop();
}

void GLVideoWidget::paintEvent(QPaintEvent *e)
{
    /*Reimplement an empty paintEvent function. This function is automatically
    *called by the main thread everytime the window is resized. The default
    *implementation would in turn call makeCurrent() which will cause a warning
    *"QGLContext::makeCurrent(): Failed." because the context is active in
    *GLThread where all the drawing should be done. This reimplementation should
    *therefore remove the warnings.*/
    Q_UNUSED(e)
}

void GLVideoWidget::onDrawFrame(unsigned char* imBuf)
{
    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)));
    glt_->drawFrame(imBuf, chunkAttrib);
    frames_++;
}

void GLVideoWidget::updateFPS()
{
    videoDialog_->updateFPS(frames_);
    frames_ = 0;
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

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

void GLVideoWidget::setVideoWidth(int newVal)
{
    glt_->setVideoWidth(newVal);
}

void GLVideoWidget::resizeEvent(QResizeEvent *e)
{
    glt_->resizeGL(e->size().width(), e->size().height());
}

void GLVideoWidget::setOutputDevice(OutputDevice::PortType portType)
{
    glt_->setOutputDevice(portType);
}

VideoDialog* GLVideoWidget::videoDialog()
{
    return videoDialog_;
}
