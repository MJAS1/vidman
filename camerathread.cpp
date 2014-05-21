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
#include <QPixmap>

#include "camerathread.h"
#include "config.h"

using namespace std;


CameraThread::CameraThread(cv::VideoCapture* capCam, CycDataBuffer* cycBuf) :
    capCam(capCam), trigCode(0), cycBuf(cycBuf)
{
    shouldStop = false;
    detectMotionEvent = NULL;

    if(!capCam->set(CV_CAP_PROP_FPS, settings.fps))
    {
        std::cerr << "Could not set framerate" << std::endl;
        abort();
    }

    if(settings.flip)
        preEvents.append(new FlipEvent(0, 0));

    if(settings.turnAround)
        preEvents.append(new RotateEvent(0, 180, 0));

    if(settings.fixPoint)
    {
        cv::Mat fixImg = cv::imread("./img/fixPoint.png", CV_LOAD_IMAGE_UNCHANGED);

        if(!fixImg.empty())
            preEvents.append(new ImageEvent(0, cv::Point2i((VIDEO_WIDTH-fixImg.cols)/2, (VIDEO_HEIGHT-fixImg.rows)/2), fixImg, 0));
        else
            std::cerr << "Couldn't load fixPoint.png" << std::endl;
    }
}


CameraThread::~CameraThread()
{
    if(detectMotionEvent)
        delete detectMotionEvent;
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

    *capCam >> frame;
    motionDetector.updateBackground(frame);

    // Start the acquisition loop
    while (!shouldStop)
    {

        *capCam >> frame;
        if (frame.empty())
        {
            std::cerr << "Error dequeuing a frame" << std::endl;
            abort();
        }

        mutex.lock();

        motionDetector.updateFrame(frame);

        applyEvents(&preEvents);

        clock_gettime(CLOCK_REALTIME, &timestamp);

		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

        if(detectMotionEvent)
        {
            if(motionDetector.movementDetected())
            {
                trigCode = detectMotionEvent->getTrigCode();
                delete detectMotionEvent;
                detectMotionEvent = NULL;
            }
        }

        applyEvents(&events);
        cv::cvtColor(frame, frame, CV_BGR2RGB);

        chunkAttrib.log = new char[log.size()+1];
        strcpy(chunkAttrib.log, log.toStdString().c_str());
        chunkAttrib.logSize = log.size();
        log.clear();

        chunkAttrib.trigCode = trigCode;

        trigCode = 0;

        mutex.unlock();


        chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * 3;
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

void CameraThread::applyEvents(const EventContainer<VideoEvent*> *ev)
{
    for(EventContainer<VideoEvent*>::ConstIterator iter = ev->begin(); iter != ev->end(); iter++)
    {
        (*iter)->apply(frame);
    }
}

void CameraThread::addVideoEvent(VideoEvent *ev)
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
        events.prepend(ev);
    else
        events.append(ev);

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

    if(!trigCode)
        trigCode = ev->getTrigCode();

    log.append(ev->getLog());

    delete ev;

    mutex.unlock();
}

void CameraThread::pause()
{
    mutex.lock();

    for(EventContainer<VideoEvent*>::Iterator iter = events.begin(); iter != events.end(); iter++)
        (*iter)->pause();

    mutex.unlock();
}

void CameraThread::unpause()
{
    mutex.lock();

    for(EventContainer<VideoEvent*>::Iterator iter = events.begin(); iter != events.end(); iter++)
        (*iter)->unpause();

    mutex.unlock();
}

void CameraThread::detectMotion(Event *ev)
{
    mutex.lock();

    detectMotionEvent = ev;
    motionDetector.startTracking();

    mutex.unlock();
}

void CameraThread::updateBackground()
{
    cv::Mat mat;
    *capCam >> mat;
    motionDetector.updateBackground(mat);
}
