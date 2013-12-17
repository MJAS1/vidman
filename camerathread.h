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
#include <QMutex>
#include <QString>
#include "stoppablethread.h"
#include "cycdatabuffer.h"
#include "event.h"
#include "eventcontainer.h"
#include "settings.h"

//! This thread acquires, timestamps and manipulates frames for a single openCV video camera.
class CameraThread : public StoppableThread
{
public:
    CameraThread(cv::VideoCapture* capCam, CycDataBuffer* cycBuf, bool color);
	virtual ~CameraThread();
    void clearEvents();

    void pause();
    void unpause();

public slots:
    void addEvent(Event *ev);
    void removeEvent(RemoveEvent *ev);

protected:
	virtual void stoppableRun();

private:
    cv::VideoCapture*           capCam;
    cv::Mat                     frame;

    QMutex                      mutex;
    QString                     log;

    bool                        color;
    int                         trigCode;

    CycDataBuffer*              cycBuf;
    EventContainer              events;
    Settings                    settings;

    void                        applyEvents();
};

#endif /* CAMERATHREAD_H_ */
