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
#include "eventcontainer.h"
#include "settings.h"
#include "motiondetector.h"
#include "videoevent.h"
#include "camera.h"

//! This thread acquires, timestamps and manipulates frames for a single openCV video camera.
class CameraThread : public StoppableThread
{
public:
    explicit CameraThread(CycDataBuffer* cycBuf);
	virtual ~CameraThread();

    void    clearEvents();
    void    pause();
    void    unpause();
    void    updateBackground();

    Camera& camera();

public slots:
    void    addVideoEvent(VideoEvent *ev);
    void    removeEvent(RemoveEvent *ev);
    void    detectMotion(Event *ev);

protected:
    virtual void stoppableRun();

private:
    CameraThread(const CameraThread&);
    CameraThread& operator=(const CameraThread&);

    CycDataBuffer*                      cycBuf_;
    Camera                              cam_;
    cv::Mat                             frame_;

    QMutex                              mutex_;
    QString                             log_;

    int                                 trigCode_;
    bool                                isDetectingMotion_;
    bool                                shouldUpdateBg;

    Event*                              detectMotionEvent_;
    EventContainer<VideoEvent*>         events_, preEvents_;
    Settings                            settings_;
    MotionDetector                      motionDetector_;
};

#endif /* CAMERATHREAD_H_ */
