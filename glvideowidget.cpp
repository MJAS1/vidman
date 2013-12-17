/*
 * glvideowidget.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <iostream>
#include <math.h>
#include <QElapsedTimer>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <unistd.h>
#include "videodialog.h"
#include "config.h"
#include "glvideowidget.h"
#include "cycdatabuffer.h"

using namespace std;


GLVideoWidget::GLVideoWidget(const QGLFormat& format, VideoDialog* parent)
    : QGLWidget(format, parent), fpsTimer(new QTimer(this)), frames(0), videoWidth(VIDEO_WIDTH)
{
    //setAutoBufferSwap(false);
    color = true;

    // Allocate memory. Since we do not know whether the image is going to BW
    // or color, allocate for color since it requires more memory.
    imBuf = (char*)malloc(VIDEO_HEIGHT*VIDEO_WIDTH*3);
    if (!imBuf)
    {
        cerr << "Error allocating memory" << endl;
        exit(EXIT_FAILURE);
    }

    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(countFPS()));
    fpsTimer->start(1000);


}


GLVideoWidget::~GLVideoWidget()
{
    free(imBuf);
}


void GLVideoWidget::initializeGL()
{
    GLuint  texture;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}


void GLVideoWidget::resizeGL(int _w, int _h)
{
    int dispW;
    int dispH;

    // Change the viewport to preserve the aspect ratio.
    // Compute new height corresponding to the current width and new width
    // corresponding to the current heigh and see which one fits.
    dispH = int(floor((_w / float(videoWidth)) * VIDEO_HEIGHT));
    dispW = int(floor((_h / float(VIDEO_HEIGHT)) * videoWidth));

    if(dispH <= _h)
    {
        glViewport(0, (_h - dispH) / 2, _w, dispH);
    }
    else if(dispW <= _w)
    {
        glViewport((_w - dispW) / 2, 0, dispW, _h);
    }
    else
    {
        cerr << "Internal error while computing the viewport size" << endl;
        abort();
    }
}


void GLVideoWidget::onDrawFrame(unsigned char* imBuf, int logSize)
{
    glClear( GL_COLOR_BUFFER_BIT);

    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromAscii((char*)(imBuf - logSize-1));

    makeCurrent();


    glTexImage2D(GL_TEXTURE_2D, 0, (color ? GL_RGB8 : GL_LUMINANCE8), VIDEO_WIDTH, VIDEO_HEIGHT, 0, (color ? GL_RGB : GL_LUMINANCE), GL_UNSIGNED_BYTE, (GLubyte*)imBuf);


    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0); glVertex2f(-1.0,+1.0);
    glTexCoord2f(1.0,0.0); glVertex2f(+1.0,+1.0);
    glTexCoord2f(1.0,1.0); glVertex2f(+1.0,-1.0);
    glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0);
    glEnd();

    updateGL();

    frames++;

    dynamic_cast<VideoDialog*>(parent())->sendTrigSignal(chunkAttrib.trigCode);

    //Write to log file. Parent widget must be a VideoDialog.
    if(logSize)
        dynamic_cast<VideoDialog*>(parent())->writeToLogFile(log);

}

void GLVideoWidget::countFPS()
{
    int fps = frames;
    frames = 0;
    dynamic_cast<VideoDialog*>(parent())->setFPS(fps);
}

void GLVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(isFullScreen())
    {
        setWindowFlags(Qt::Widget);
        showNormal();
    }
    else
    {
        setWindowFlags(Qt::Window);
        showFullScreen();
    }
}

void GLVideoWidget::setVideoWidth(int newVal)
{
    videoWidth = newVal;
    resizeGL(width(), height());
}
