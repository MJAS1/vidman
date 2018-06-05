#include <iostream>
#include <jpeglib.h>
#include <stdlib.h>

#include "config.h"
#include "videofilewriter.h"

using namespace std;


VideoFileWriter::VideoFileWriter(CycDataBuffer* cycBuf, const char* path,
                                 QObject *parent) :
    FileWriter(cycBuf, path, "_video", "vid", parent)
{
    uint32_t ver = VIDEO_FILE_VERSION;

    bufLen_ = strlen(MAGIC_VIDEO_STR) + sizeof(uint32_t);
    buf_ = new unsigned char[bufLen_];

    if(!buf_)
    {
        cerr << "Error allocating memory!" << endl;
        abort();
    }

    // String identifying the file type
    memcpy(buf_, MAGIC_VIDEO_STR, strlen(MAGIC_VIDEO_STR));
    // Version of file format
    memcpy(buf_ + strlen(MAGIC_VIDEO_STR), &ver, sizeof(uint32_t));
}

VideoFileWriter::~VideoFileWriter()
{
    delete []buf_;
}

unsigned char* VideoFileWriter::getHeader(int* len)
{
    *len = bufLen_;
    return(buf_);
}
