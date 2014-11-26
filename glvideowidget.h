/*
 * glvideowidget.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef GLVIDEOWIDGET_H_
#define GLVIDEOWIDGET_H_

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QMutex>
#include <QWaitCondition>
#include <jpeglib.h>
#include <QTimer>
#include "stoppablethread.h"
#include "common.h"
#include "outputdevice.h"
#include "glthread.h"

class VideoDialog;

/* GLVideoWidget is used as the widget where each frame is drawn using OpenGL. The actual
 * drawing and swapping of the buffers are done in a separate thread (GLThread). This is because
 * when using 60 FPS for buffer swapping, it is enough to stall the UI thread especially with vsync on. */

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, OutputDevice *trigPort, VideoDialog* parent=0);
    virtual ~GLVideoWidget();

    void setVideoWidth(int newVal);

    GLThread glt;

public slots:
    void onDrawFrame(unsigned char* _jpegBuf, int );

protected:
    void initializeGL();
    void resizeGL(int _w, int _h);
    void paintGL();
    void mouseDoubleClickEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);

private:

    QTimer      *fpsTimer;

    char        *imBuf;
    int         frames;
    int         videoWidth;

    QGLShaderProgram shaderProgram;
    QVector<QVector2D> vertices;
    QVector<QVector2D> textureCoordinates;

    OutputDevice *trigPort;

    QMutex mutex;

private slots:
    void countFPS();
};

#endif /* GLVIDEOWIDGET_H_ */
