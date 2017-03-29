/*
 * videocompressorthread.h
 *
 *  Created on: Jan 14, 2010
 *      Author: meg2meg
 */

#ifndef VIDEOCOMPRESSORTHREAD_H_
#define VIDEOCOMPRESSORTHREAD_H_

#include "stoppablethread.h"
#include "cycdatabuffer.h"

class VideoCompressorThread : public StoppableThread
{
public:
    VideoCompressorThread(CycDataBuffer* _inpBuf, CycDataBuffer* _outBuf,
                          int _jpgQuality, QObject* parent = 0);
    virtual ~VideoCompressorThread();

protected:
    virtual void stoppableRun();

private:
    CycDataBuffer*	inpBuf;
    CycDataBuffer*	outBuf;
    int				jpgQuality;
};

#endif /* VIDEOCOMPRESSORTHREAD_H_ */
