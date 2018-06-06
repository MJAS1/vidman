/*
 * glvideowidget.h
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

#ifndef GLVIDEOWIDGET_H_
#define GLVIDEOWIDGET_H_

#include <QGLWidget>

class VideoDialog;

/*!
 * GLVideoWidget is used as the widget where each frame is drawn, using OpenGL.
 * The actual drawing and swapping of the buffers are done in a separate thread
 * by GLWorker. This is because when using 60 FPS for buffer swapping, it is
 * enough to stall the UI thread and make it unresponsive especially when using
 * vsync.
 */

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, QWidget* parent);

signals:
    void    resize(int w, int h);
    void    pause();

protected:
    void    mouseDoubleClickEvent(QMouseEvent *e);
    void    keyPressEvent(QKeyEvent* ev);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*e);
    QSize   sizeHint() const {return QSize(640, 480);}
};

#endif /* GLVIDEOWIDGET_H_ */
