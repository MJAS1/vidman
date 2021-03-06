/*
 * glvideowidget.cpp
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QResizeEvent>
#include <QThread>

#include "glvideowidget.h"

using namespace std;

GLVideoWidget::GLVideoWidget(const QGLFormat& format, QWidget* parent)
    : QGLWidget(format, parent)
{
    // Important to manually swap the framebuffers for syncing trigger signals.
    setAutoBufferSwap(false);
}

void GLVideoWidget::paintEvent(QPaintEvent *)
{
    /*
     * Reimplement an empty paintEvent function. This function is automatically
     * called by the main thread everytime the window is resized. The default
     * implementation would in turn call makeCurrent() which will cause a
     * warning "QGLContext::makeCurrent(): Failed." because the context is
     * active in another GLThread where all the drawing should be done. This
     * reimplementation should therefore remove the warnings.
     */
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    /*
     * After migrating from Qt4 to Qt5, a problem appeared. When coming back to
     * normal state from fullscreen, the widget showed only a blank screen. This
     * is probably due to some internal bug in Qt code. This hack made the
     * problem disappear, at least most of the time.
     */
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
}
