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
