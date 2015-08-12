/*
 * glvideowidget.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef GLVIDEOWIDGET_H_
#define GLVIDEOWIDGET_H_

#include <QGLWidget>
#include <QTimer>
#include <QThread>
#include "outputdevice.h"
#include "glworker.h"

class VideoDialog;

/* GLVideoWidget is used as the widget where each frame is drawn using OpenGL.
 * The actual drawing and swapping of the buffers are done in a separate thread.
 * This is because when using 60 FPS for buffer swapping, it is enough to stall
 * the UI thread and make it unresponsive especially with vsync on. */

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, VideoDialog* parent=0);
    virtual ~GLVideoWidget();

    void setVideoWidth(int newVal);
    void setOutputDevice(OutputDevice::PortType portType);

    VideoDialog* videoDialog();

public slots:
    void    onDrawFrame();

signals:
    void    drawFrame(unsigned char* imBuf);
    void    resize(int w, int h);

protected:
    void    mouseDoubleClickEvent(QMouseEvent *e);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*e);
    QSize   sizeHint() const {return QSize(640, 480);}

private:
    GLVideoWidget(const GLVideoWidget&);
    GLVideoWidget& operator=(const GLVideoWidget&);

    QTimer          fpsTimer_;

    int             frames_;
    int             videoWidth_;

    VideoDialog*    videoDialog_;
    QThread         glthread_;
    GLWorker        glworker_;

private slots:
    void updateFPS();
};

#endif /* GLVIDEOWIDGET_H_ */
