/*
 * glvideowidget.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef GLVIDEOWIDGET_H_
#define GLVIDEOWIDGET_H_

#include <QGLWidget>
#include <jpeglib.h>

class GLVideoWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLVideoWidget(const QGLFormat& format, QWidget* parent=0);
    virtual ~GLVideoWidget();
    volatile bool color;

public slots:
    void onDrawFrame(unsigned char* _jpegBuf, int );

protected:
    void initializeGL();
    void resizeGL(int _w, int _h);

private:
    char*		imBuf;

    int fileDescriptor;
};

#endif /* GLVIDEOWIDGET_H_ */
