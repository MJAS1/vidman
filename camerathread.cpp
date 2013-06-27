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

		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

        mutex.lock();
        applyEvents();
        cv::cvtColor(frame, frame, CV_BGR2RGB);

        if(logSemaphore.tryAcquire())
        {
            logSemaphore.acquire(logSemaphore.available());
            chunkAttrib.log = new char[log.size()+1];
            strncpy(chunkAttrib.log, log.toStdString().c_str(), log.size()+1);
            chunkAttrib.logSize = log.size();
            log.clear();
        }
        else
            chunkAttrib.logSize = 0;
        mutex.unlock();

		chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1);
        chunkAttrib.timestamp = msec;

        cycBuf->insertChunk((uchar*)frame.data, chunkAttrib);


        if(chunkAttrib.logSize)
            delete []chunkAttrib.log;

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
    for(EventContainer::iterator iter = events.begin(); iter != events.end(); iter++)
    {
        (*iter)->apply(frame);
    }
}

void CameraThread::addEvent(Event *ev)
{
    mutex.lock();
    if (ev->getType() == EVENT_FADEIN || ev->getType() == EVENT_FADEOUT)
    {
        events.removeType(EVENT_FADEIN);
        events.removeType(EVENT_FADEOUT);
    }

    if(ev->getType() == EVENT_FLIP) log.append("Flip event added ");
    else if(ev->getType() == EVENT_FADEIN) log.append("Fadein event added ");
    else if(ev->getType() == EVENT_FADEOUT) log.append("Fadeout event added ");
    else if(ev->getType() == EVENT_IMAGE) log.append("Image event added ");
    else if(ev->getType() == EVENT_TEXT) log.append("Text event added ");
    else if(ev->getType() == EVENT_ROTATE) log.append("Rotate event added ");
    else if(ev->getType() == EVENT_FREEZE) log.append("Freeze event added ");

    logSemaphore.release();

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

    if(ev->getRemoveType() == EVENT_FLIP) log.append("Flip event removed ");
    else if(ev->getRemoveType() == EVENT_FADEIN) log.append("Fadein event removed ");
    else if(ev->getRemoveType() == EVENT_FADEOUT) log.append("Fadeout event removed ");
    else if(ev->getRemoveType() == EVENT_IMAGE) log.append("Image event removed ");
    else if(ev->getRemoveType() == EVENT_TEXT) log.append("Text event removed ");
    else if(ev->getRemoveType() == EVENT_ROTATE) log.append("Rotate event removed ");
    else if(ev->getRemoveType() == EVENT_FREEZE) log.append("Freeze event removed ");
    else log.append(QString("Event ID %1 removed ").arg(ev->getRemoveId()));
    logSemaphore.release();

    mutex.unlock();
}

