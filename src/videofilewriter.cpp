/*
 * videofilewriter.cpp
 *
 * Original file:
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * Modifications:
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
