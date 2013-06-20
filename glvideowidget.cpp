/*
 * glvideowidget.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <iostream>
#include <math.h>

#include "config.h"
#include "glvideowidget.h"
#include "cycdatabuffer.h"

using namespace std;


GLVideoWidget::GLVideoWidget(QWidget* parent)
	: QGLWidget(parent)
{
	color = false;

	// Allocate memory. Since we do not know whether the image is going to BW
	// or color, allocate for color since it requires more memory.
	imBuf = (char*)malloc(VIDEO_HEIGHT*VIDEO_WIDTH*3);
	if (!imBuf)
	{
		cerr << "Error allocating memory" << endl;
		exit(EXIT_FAILURE);
	}

	row_pointer[0] = (unsigned char *)malloc(VIDEO_WIDTH*3);
	if(!(row_pointer[0]))
	{
		cerr << "Error allocating memory" << endl;
		exit(EXIT_FAILURE);
	}
}


GLVideoWidget::~GLVideoWidget()
{
	free(imBuf);
	free(row_pointer[0]);
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


void GLVideoWidget::onDrawFrame(unsigned char* _jpegBuf)
{
	int 		i;
	ChunkAttrib chunkAttrib;

	chunkAttrib = *((ChunkAttrib*)(_jpegBuf-sizeof(ChunkAttrib)));

	//---------------------------------------------------------------------
	// Decompress JPEG image to memory
	//

	// initialize JPEG
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, _jpegBuf, chunkAttrib.chunkSize);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);


	for (i=0; i<VIDEO_HEIGHT; i++)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		memcpy((char*)imBuf + i*VIDEO_WIDTH*(color?3:1), row_pointer[0], VIDEO_WIDTH*(color?3:1));
	}

	// clean-up JPEG
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	//
	// done decompressing
	//---------------------------------------------------------------------

	makeCurrent();
    glTexImage2D(GL_TEXTURE_2D, 0, (color ? GL_RGB8 : GL_LUMINANCE8), VIDEO_WIDTH, VIDEO_HEIGHT, 0, (color ? GL_RGB : GL_LUMINANCE), GL_UNSIGNED_BYTE, (GLubyte*)imBuf);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2d(-1.0,+1.0);
    glTexCoord2d(1.0,0.0); glVertex2d(+1.0,+1.0);
    glTexCoord2d(1.0,1.0); glVertex2d(+1.0,-1.0);
    glTexCoord2d(0.0,1.0); glVertex2d(-1.0,-1.0);
    glEnd();

    updateGL();
}
