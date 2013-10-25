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


GLVideoWidget::GLVideoWidget(const QGLFormat& format, QWidget* parent)
    : QGLWidget(format, parent), fileDescriptor(-1)
{
    color = true;

    // Allocate memory. Since we do not know whether the image is going to BW
    // or color, allocate for color since it requires more memory.
    imBuf = (char*)malloc(VIDEO_HEIGHT*VIDEO_WIDTH*3);
    if (!imBuf)
    {
        cerr << "Error allocating memory" << endl;
        exit(EXIT_FAILURE);
    }

    //Open USB port for trigger signals
    if((fileDescriptor = ::open("/dev/ttyUSB0", O_RDWR)) < 1)
    {
        QMessageBox msgBox;
        msgBox.setText("Couldn't open USB port for trigger signals.");
        msgBox.exec();
    }

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
    dispH = int(floor((_w / float(VIDEO_WIDTH)) * VIDEO_HEIGHT));
    dispW = int(floor((_h / float(VIDEO_HEIGHT)) * VIDEO_WIDTH));

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

    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(imBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromAscii((char*)(imBuf - logSize-1));


    makeCurrent();
    glTexImage2D(GL_TEXTURE_2D, 0, (color ? GL_RGB8 : GL_LUMINANCE8), VIDEO_WIDTH, VIDEO_HEIGHT, 0, (color ? GL_RGB : GL_LUMINANCE), GL_UNSIGNED_BYTE, (GLubyte*)imBuf);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2d(-1.0,+1.0);
    glTexCoord2d(1.0,0.0); glVertex2d(+1.0,+1.0);
    glTexCoord2d(1.0,1.0); glVertex2d(+1.0,-1.0);
    glTexCoord2d(0.0,1.0); glVertex2d(-1.0,-1.0);
    glEnd();

    updateGL();

    //Set modem control line according to the trigcode
    if(fileDescriptor >= 1)
    {
        if(ioctl(fileDescriptor, TIOCMSET, &chunkAttrib.trigCode) == -1)
            fprintf(stderr, "Cannot open port: %s\n", strerror(errno));
    }

    //Write to log file. Parent widget must be a VideoDialog.
    if(logSize)
        dynamic_cast<VideoDialog*>(parent())->writeToLogFile(log);
}

