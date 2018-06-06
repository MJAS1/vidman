/*
 * videofilewriter.h
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

#ifndef VIDEOFILEWRITER_H_
#define VIDEOFILEWRITER_H_

#include "filewriter.h"

class VideoFileWriter : public FileWriter
{
public:
    VideoFileWriter(CycDataBuffer* cycBuf, const char* path,
    QObject* parent = nullptr);
    virtual ~VideoFileWriter();

protected:
    virtual unsigned char* getHeader(int* _len);

private:
    VideoFileWriter(const VideoFileWriter&);
    VideoFileWriter& operator=(const VideoFileWriter&);

    int				bufLen_;
    unsigned char*	buf_;
};

#endif /* VIDEOFILEWRITER_H_ */
