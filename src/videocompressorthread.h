/*
 * videocompressorthread.h
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
