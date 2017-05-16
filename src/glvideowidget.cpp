#include <QResizeEvent>
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
    /* TODO: Fix black screen issue after exiting full screen. This problem was
     * introduced after upgrading to Qt5 from Qt4.8. */
    if(isFullScreen()) {
        setWindowFlags(Qt::Widget);
        showNormal();
        parentWidget()->show();
    }
    else {
        setWindowFlags(Qt::Window);
        parentWidget()->close();
        showFullScreen();
    }
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
