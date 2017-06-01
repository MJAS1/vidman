#include <QResizeEvent>
#include <QThread>
#include "glvideowidget.h"

using namespace std;

GLVideoWidget::GLVideoWidget(const QGLFormat& format, QWidget* parent)
    : QGLWidget(format, parent)
{
    //Important to manually swap the framebuffers for syncing trigger signals.
    setAutoBufferSwap(false);
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

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    /* After updating Qt4 to Qt5 a problem appeared. When coming back to normal
     * state from fullscreen, the widget showed only a blank black screen.
     * This is probably due to some internal bug in Qt code. For some very
     * strange reason the following piece of code made the problem disappear, at
     * least for most of the time. */
    setWindowFlags(windowFlags() ^ Qt::Window);
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    thread()->msleep(20);
    setWindowFlags(windowFlags() ^ Qt::Window);
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    setWindowFlags(windowFlags() ^ Qt::Window);
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    show();
    parentWidget()->repaint();
    parentWidget()->update();
}

void GLVideoWidget::resizeEvent(QResizeEvent *e)
{
    emit resize(e->size().width(), e->size().height());
}

void GLVideoWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Space)
        emit pause();
    else if(ev->key() == Qt::Key_Plus)
        emit increaseAspectRatio();
    else if(ev->key() == Qt::Key_Minus)
        emit decreaseAspectRatio();
}
