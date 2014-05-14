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
#include <jpeglib.h>
#include <QTimer>
#include "stoppablethread.h"
#include "common.h"
#include "outputdevice.h"

class VideoDialog;

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, OutputDevice *trigPort, VideoDialog* parent=0);
    virtual ~GLVideoWidget();
    volatile bool color;

    void setVideoWidth(int newVal);

public slots:
    void onDrawFrame(unsigned char* _jpegBuf, int );

protected:
    void initializeGL();
    void resizeGL(int _w, int _h);
    void paintGL();
    void mouseDoubleClickEvent(QMouseEvent *e);

private:

    QTimer      *fpsTimer;

    char        *imBuf;
    int         frames;
    int         videoWidth;

    QGLShaderProgram shaderProgram;
    QVector<QVector2D> vertices;
    QVector<QVector2D> textureCoordinates;

    OutputDevice *trigPort;

private slots:
    void countFPS();
};

#endif /* GLVIDEOWIDGET_H_ */
