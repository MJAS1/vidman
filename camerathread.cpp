/*
 * camerathread.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */
#include <iostream>
#include <sched.h>
#include <time.h>
#include <QCoreApplication>
#include <stdlib.h>
#include <QTimer>

#include "camerathread.h"
#include "config.h"

using namespace std;


CameraThread::CameraThread(cv::VideoCapture* capCam, CycDataBuffer* _cycBuf, bool _color) :
    capCam(capCam), cycBuf(_cycBuf), color(_color)
{
    shouldStop = false;
    if(!capCam->set(CV_CAP_PROP_FPS, 30))
    {
        std::cerr << "Could not set framerate" << std::endl;
        abort();
    }
}


CameraThread::~CameraThread()
{
}


void CameraThread::stoppableRun()
{
    struct sched_param		sch_param;
	struct timespec			timestamp;
	uint64_t				msec;
	ChunkAttrib				chunkAttrib;

    /*-----------------------------------------------------------------------
     *  have the camera start sending us data
     *-----------------------------------------------------------------------*/

    // Set priority
    sch_param.sched_priority = CAM_THREAD_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &sch_param))
    {
        std::cerr << "Cannot set camera thread priority. Continuing nevertheless, but don't blame me if you experience any strange problems." << std::endl;
    }

    // Start the acquisition loop
    while (!shouldStop)
    {
        *capCam >> frame;

        if (frame.empty())
        {
            std::cerr << "Error dequeuing a frame" << std::endl;
            abort();
        }

        clock_gettime(CLOCK_REALTIME, &timestamp);

        applyEvents();

        cv::cvtColor(frame, frame, CV_BGR2RGB);

		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

		chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1);
        chunkAttrib.timestamp = msec;
        cycBuf->insertChunk((uchar*)frame.data, chunkAttrib);
    }

}

void CameraThread::clearEvents()
{
    mutex.lock();
    events.clear();
    mutex.unlock();
}

void CameraThread::applyEvents()
{
    mutex.lock();
    for(EventContainer::iterator iter = events.begin(); iter != events.end(); iter++)
    {
        (*iter)->apply(frame);
    }
    mutex.unlock();

}

void CameraThread::addEvent(Event *ev)
{
    mutex.lock();
    if (ev->getType() == EVENT_FADEIN || ev->getType() == EVENT_FADEOUT)
    {
        events.removeType(EVENT_FADEIN);
        events.removeType(EVENT_FADEOUT);
    }

    events.push_back(ev);
    mutex.unlock();
}

void CameraThread::removeEvent(RemoveEvent *ev)
{
    mutex.lock();
    if(ev->getRemoveType() != EVENT_NULL)
        events.removeType(ev->getRemoveType());
    else
        events.removeId(ev->getRemoveId());
    mutex.unlock();
}

