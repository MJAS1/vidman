#ifndef VIDEOCOMPRESSORTHREAD_H_
#define VIDEOCOMPRESSORTHREAD_H_

#include "stoppablethread.h"
#include "cycdatabuffer.h"

/*!
 * This thread compressess frames to jpg format.
 */
class VideoCompressorThread : public StoppableThread
{
public:
    VideoCompressorThread(CycDataBuffer* inpBuf, CycDataBuffer* outBuf,
                          int jpgQuality, QObject* parent = nullptr);
    virtual ~VideoCompressorThread();

protected:
    virtual void stoppableRun();

private:
    CycDataBuffer*	inpBuf_;
    CycDataBuffer*	outBuf_;
    int				jpgQuality_;
};

#endif /* VIDEOCOMPRESSORTHREAD_H_ */
