/*
 * videofilewriter.h
 *
 *  Created on: Jul 1, 2010
 *      Author: andrey
 */

#ifndef VIDEOFILEWRITER_H_
#define VIDEOFILEWRITER_H_

#include "filewriter.h"

class VideoFileWriter : public FileWriter
{
public:
    VideoFileWriter(CycDataBuffer* cycBuf, const char* path, QObject* parent = 0);
    virtual ~VideoFileWriter();

protected:
    virtual unsigned char* getHeader(int* _len);

private:
    int				bufLen;
    unsigned char*	buf;
};

#endif /* VIDEOFILEWRITER_H_ */
