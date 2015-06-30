/*
 * camerathread.cpp
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#include <iostream>
#include <sched.h>
#include <time.h>
#include <fstream>
#include <QDebug>
#include <QImage>

#include "camerathread.h"
#include "config.h"

using namespace std;


CameraThread::CameraThread(CycDataBuffer* cycBuf, Camera &cam, QObject* parent) :
    StoppableThread(parent), cycBuf_(cycBuf), cam_(cam), trigCode_(0), shouldUpdateBg_(true)
{
    //Setup preEvents for default processing each frame before actual manipulation
    if(settings_.flip)
        preEvents_.append(new FlipEvent(0, 0));

    if(settings_.turnAround)
        preEvents_.append(new RotateEvent(0, 180, 0));

    if(settings_.fixPoint) {
        //fixPoint.png is stored in qt resource file, so it needs to be loaded to QImage first
        QImage fixImg(":/img/fixPoint.png");
        cv::Mat fixMat = cv::Mat(fixImg.height(), fixImg.width(), CV_8UC4, fixImg.bits(), fixImg.bytesPerLine()).clone();
        cv::cvtColor(fixMat, fixMat, CV_RGBA2BGRA);

        if(!fixMat.empty())
            preEvents_.append(new ImageEvent(0, cv::Point2i((VIDEO_WIDTH-fixMat.cols)/2, (VIDEO_HEIGHT-fixMat.rows)/2), fixMat, 0));
        else
            std::cerr << "Couldn't load fixPoint.png" << std::endl;
    }
}


void CameraThread::stoppableRun()
{
    struct sched_param		sch_param;
	struct timespec			timestamp;
    uint64_t                msec;
	ChunkAttrib				chunkAttrib;

    cam_.setFPS(settings_.fps);

    /*-----------------------------------------------------------------------
     *  have the camera start sending us data
     *-----------------------------------------------------------------------*/

    // Set priority
    sch_param.sched_priority = CAM_THREAD_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &sch_param))
        std::cerr << "Cannot set camera thread priority. Continuing nevertheless, but don't blame me if you experience any strange problems." << std::endl;

    // Start the acquisition loop
    while (!shouldStop) {
        cam_ >> frame_;
        if (frame_.empty()) {
            std::cerr << "Error dequeuing a frame" << std::endl;
            abort();
        }

        mutex_.lock();

        //If necessary, update motionDetector background image for substraction
        if(shouldUpdateBg_) {
            motionDetector_.updateBackground(frame_);
            shouldUpdateBg_ = false;
        }
        if(motionDetector_.movementDetected(frame_)) {
            trigCode_ = motionDetector_.getEventTrigCode();
            log_.append(motionDetector_.getEventLog());
        }

        preEvents_.applyEvents(frame_);

        clock_gettime(CLOCK_REALTIME, &timestamp);
		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

        events_.applyEvents(frame_);
        cv::cvtColor(frame_, frame_, CV_BGR2RGB);

        chunkAttrib.log = new char[log_.size()+1];
        strcpy(chunkAttrib.log, log_.toStdString().c_str());
        chunkAttrib.logSize = log_.size();
        log_.clear();

        chunkAttrib.trigCode = trigCode_;
        trigCode_ = 0;

        chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * 3;
        chunkAttrib.timestamp = msec;

        cycBuf_->insertChunk(frame_.data, chunkAttrib);

        delete []chunkAttrib.log;

        //Emit the background substracted pixmap of the hands to MainWindow for
        //motionDetector_ label.
        emit motionDetectorPixmap(motionDetector_.foregroundPixmap());

        mutex_.unlock();
    }

}

void CameraThread::clearEvents()
{
    mutex_.lock();
    events_.clear();
    mutex_.unlock();
}

void CameraThread::handleEvent(Event *ev)
{
    mutex_.lock();

    if(ev->getType() == Event::EVENT_DETECT_MOTION) {
        motionDetector_.startTracking(ev);
    }
    else {
        trigCode_ = ev->getTrigCode();
        log_.append(ev->getLog());
        events_.insert(ev);
    }
    mutex_.unlock();
}

void CameraThread::pause()
{
    mutex_.lock();
    events_.pauseEvents();
    mutex_.unlock();
}

void CameraThread::unpause()
{
    mutex_.lock();
    events_.unpauseEvents();
    mutex_.unlock();
}

void CameraThread::updateBackground()
{
    mutex_.lock();
    shouldUpdateBg_ = true;
    mutex_.unlock();
}
