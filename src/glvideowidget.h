#ifndef GLVIDEOWIDGET_H_
#define GLVIDEOWIDGET_H_

#include <QGLWidget>

class VideoDialog;

/*!
 * GLVideoWidget is used as the widget where each frame is drawn using OpenGL.
 * The actual drawing and swapping of the buffers are done in a separate thread.
 * This is because when using 60 FPS for buffer swapping, it is enough to stall
 * the UI thread and make it unresponsive especially with vsync on.
*/

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, VideoDialog* parent=nullptr);

signals:
    void    resize(int w, int h);
    void    pause();
    void    increaseAspectRatio();
    void    decreaseAspectRatio();

protected:
    void    mouseDoubleClickEvent(QMouseEvent *e);
    void    keyPressEvent(QKeyEvent* ev);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*e);
    QSize   sizeHint() const {return QSize(640, 480);}

private:
    int             videoWidth_;
    VideoDialog*    videoDialog_;
};

#endif /* GLVIDEOWIDGET_H_ */
