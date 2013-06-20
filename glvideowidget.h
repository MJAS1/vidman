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
	GLVideoWidget(QWidget* parent=0);
	virtual ~GLVideoWidget();
	volatile bool color;

public slots:
    void onDrawFrame(unsigned char* _jpegBuf);

protected:
    void initializeGL();
    void resizeGL(int _w, int _h);

private:
	char*		imBuf;

	// JPEG-related stuff
	struct		jpeg_decompress_struct cinfo;
	struct		jpeg_error_mgr jerr;
	JSAMPROW 	row_pointer[1];
};

#endif /* GLVIDEOWIDGET_H_ */
