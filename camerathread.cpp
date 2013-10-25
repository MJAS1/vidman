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
    capCam(capCam), color(color), trigCode(0), cycBuf(cycBuf)
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

        chunkAttrib.trigCode = trigCode;
        trigCode = 0;

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

    //Remove duplicate events of certain event types to prevent the program from slowing down
    switch(ev->getType())
    {
        case EVENT_FLIP:
            events.removeType(EVENT_FLIP);
            break;

        case EVENT_FADEIN:
            events.removeType(EVENT_FADEIN);
            events.removeType(EVENT_FADEOUT);
            break;

        case EVENT_FADEOUT:
            events.removeType(EVENT_FADEIN);
            events.removeType(EVENT_FADEOUT);
            break;

        case EVENT_ROTATE:
            events.removeType(EVENT_ROTATE);
            break;

        case EVENT_FREEZE:
            events.removeType(EVENT_FREEZE);
            break;

        default:
            break;
    }

    trigCode = ev->getTrigCode();
    log.append(ev->getLog());
    if(ev->getType() == EVENT_FREEZE)
        events.push_front(ev);
    else
        events.push_back(ev);

    mutex.unlock();
}

void CameraThread::removeEvent(RemoveEvent *ev)
{
    mutex.lock();

    //Remove all events of given type or id
    if(ev->getRemoveType())
        events.removeType(ev->getRemoveType());
    else
        events.removeId(ev->getRemoveId());

    trigCode = ev->getTrigCode();
    log.append(ev->getLog());

    delete ev;

    mutex.unlock();
}

void CameraThread::pause()
{
    mutex.lock();

    for(EventContainer::iterator iter = events.begin(); iter != events.end(); iter++)
    {
        if((*iter)->getType() == EVENT_FADEIN)
        {
            dynamic_cast<FadeInEvent*>(*iter)->pause();
            break;
        }
        else if(((*iter)->getType() == EVENT_FADEOUT))
        {
            dynamic_cast<FadeOutEvent*>(*iter)->pause();
            break;
        }
    }

    mutex.unlock();
}

void CameraThread::unpause()
{
    mutex.lock();

    for(EventContainer::iterator iter = events.begin(); iter != events.end(); iter++)
    {
        if((*iter)->getType() == EVENT_FADEIN)
        {
            dynamic_cast<FadeInEvent*>(*iter)->unpause();
            break;
        }
        else if(((*iter)->getType() == EVENT_FADEOUT))
        {
            dynamic_cast<FadeOutEvent*>(*iter)->unpause();
            break;
        }
    }

    mutex.unlock();
}

