/*
 * videofilewriter.cpp
 *
 *  Created on: Jul 1, 2010
 *      Author: andrey
 */
#include <iostream>
#include <jpeglib.h>
#include <stdlib.h>

#include "config.h"
#include "videofilewriter.h"

using namespace std;


VideoFileWriter::VideoFileWriter(CycDataBuffer* cycBuf, const char* path, QObject *parent)
    :	FileWriter(cycBuf, path, "_video", "vid", parent)
{
	uint32_t ver = VIDEO_FILE_VERSION;

	bufLen = strlen(MAGIC_VIDEO_STR) + sizeof(uint32_t);
    buf = new unsigned char[bufLen];

	if(!buf)
	{
		cerr << "Error allocating memory!" << endl;
		abort();
    }

	memcpy(buf, MAGIC_VIDEO_STR, strlen(MAGIC_VIDEO_STR));			// string identifying the file type
	memcpy(buf + strlen(MAGIC_VIDEO_STR), &ver, sizeof(uint32_t));	// version of file format
}


VideoFileWriter::~VideoFileWriter()
{
    delete []buf;
}


unsigned char* VideoFileWriter::getHeader(int* _len)
{
	*_len = bufLen;
	return(buf);
}
