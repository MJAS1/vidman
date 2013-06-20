/*
 * camerathread.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef CAMERATHREAD_H_
#define CAMERATHREAD_H_

#include <cv.h>
#include <highgui.h>

#include "stoppablethread.h"
#include "cycdatabuffer.h"
#include "event.h"
#include "eventcontainer.h"

//! This thread acquires and timestamps frames for a single libdc1394 video camera.
class CameraThread : public StoppableThread
{
public:
    CameraThread(cv::VideoCapture* capCam, CycDataBuffer* _cycBuf, bool _color);
	virtual ~CameraThread();
    void clearEvents();
    void setEvents(bool);
public slots:
    void mirrorVideo();
    void addEvent(Event *ev);
    void addImage();

protected:
	virtual void stoppableRun();

private:
    cv::VideoCapture*           capCam;
    cv::Mat                     frame;
    CycDataBuffer*              cycBuf;

    bool                        color, eventsOn;

    EventContainer              events;

    void                        applyEvents();
};

#endif /* CAMERATHREAD_H_ */
