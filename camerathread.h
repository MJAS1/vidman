/*
 * camerathread.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef CAMERATHREAD_H_
#define CAMERATHREAD_H_

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QMutex>
#include <QString>
#include "stoppablethread.h"
#include "cycdatabuffer.h"
#include "event.h"
#include "eventcontainer.h"
#include "settings.h"
#include "motiondetector.h"
#include "videoevent.h"
#include "camera.h"

//! This thread acquires, timestamps and manipulates frames for a single openCV video camera.
class CameraThread : public StoppableThread
{
public:
    CameraThread(Camera* cam, CycDataBuffer* cycBuf);
	virtual ~CameraThread();
    void clearEvents();

    void pause();
    void unpause();

    void updateBackground();

public slots:
    void addVideoEvent(VideoEvent *ev);
    void removeEvent(RemoveEvent *ev);
    void detectMotion(Event *ev);

protected:
    virtual void stoppableRun();

private:
    Camera*                                   cam;
    cv::Mat                                  frame;

    QMutex                                   mutex;
    QString                                  log;

    int                                      trigCode;
    bool                                     isDetectingMotion;
    Event*                                   detectMotionEvent;

    CycDataBuffer*                           cycBuf;
    EventContainer<VideoEvent*>              events, preEvents;
    Settings                                 settings;
    MotionDetector                           motionDetector;

    void                                     applyEvents(const EventContainer<VideoEvent*>*);
};

#endif /* CAMERATHREAD_H_ */
