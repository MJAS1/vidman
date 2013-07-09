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
#include <QTimer>

#include "camerathread.h"
#include "config.h"

using namespace std;


CameraThread::CameraThread(cv::VideoCapture* capCam, CycDataBuffer* cycBuf, bool color) :
    capCam(capCam), cycBuf(cycBuf), color(color)
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

        chunkAttrib.log = new char[log.size()+1];
        strcpy(chunkAttrib.log, log.toStdString().c_str());
        chunkAttrib.logSize = log.size();
        log.clear();

        mutex.unlock();

		chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1);
        chunkAttrib.timestamp = msec;

        cycBuf->insertChunk(frame.data, chunkAttrib);

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

    switch(ev->getType())
    {
        case EVENT_FLIP:
            events.removeType(EVENT_FLIP);
            log.append("Flip event added ");
            break;

        case EVENT_FADEIN:
            log.append("Fadein event added ");
            events.removeType(EVENT_FADEIN);
            events.removeType(EVENT_FADEOUT);
            break;

        case EVENT_FADEOUT:
            log.append("Fadeout event added ");
            events.removeType(EVENT_FADEIN);
            events.removeType(EVENT_FADEOUT);
            break;

        case EVENT_IMAGE:
            log.append("Image event added ");
            break;

        case EVENT_TEXT:
            log.append("Text event added ");
            break;

        case EVENT_ROTATE:
            events.removeType(EVENT_ROTATE);
            log.append("Rotate event added ");
            break;

        case EVENT_FREEZE:
            events.removeType(EVENT_FREEZE);
            log.append("Freeze event added ");
            break;

        default:
            break;
    }
    events.push_back(ev);

    mutex.unlock();
}

void CameraThread::removeEvent(RemoveEvent *ev)
{
    mutex.lock();

    if(ev->getRemoveType())
        events.removeType(ev->getRemoveType());
    else
        events.removeId(ev->getRemoveId());

    switch(ev->getRemoveType())
    {
        case EVENT_FLIP:
            log.append("Flip event removed ");
            break;

        case EVENT_FADEIN:
            log.append("Fadein event removed ");
            break;

        case EVENT_FADEOUT:
            log.append("Fadeout event removed ");
            break;

        case EVENT_IMAGE:
            log.append("Image event removed ");
            break;

        case EVENT_TEXT:
            log.append("Text event removed ");
            break;

        case EVENT_ROTATE:
            log.append("Rotate event removed ");
            break;

        case EVENT_FREEZE:
            log.append("Freeze event removed ");
            break;

        default:
            log.append(QString("Event ID %1 removed ").arg(ev->getRemoveId()));
            break;
    }

    delete ev;

    mutex.unlock();
}

