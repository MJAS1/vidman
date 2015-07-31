/*
 * camerathread.h
 *
 *  Created on: Jun 6, 2010
 *      Author: andrey
 */

#ifndef CAMERATHREAD_H_
#define CAMERATHREAD_H_

#include <QMutex>
#include <opencv2/opencv.hpp>
#include "event.h"
#include "stoppablethread.h"
#include "eventcontainer.h"
#include "settings.h"
#include "motiondetector.h"

class QMutex;
class Camera;
class VideoEvent;
class CycDataBuffer;

//! This thread acquires, timestamps and manipulates frames for a single openCV video camera.
class CameraThread : public StoppableThread
{
    Q_OBJECT

public:
    explicit CameraThread(CycDataBuffer* cycBuf, Camera &cam, QObject* parent = 0);

    void    clearEvents();
    void    pause();
    void    unpause();

    void    handleEvent(EventPtr ev);

signals:
    void    motionDetectorPixmap(const QPixmap&);

protected:
    virtual void stoppableRun();

private:
    CameraThread(const CameraThread&);
    CameraThread& operator=(const CameraThread&);

    CycDataBuffer*                      cycBuf_;
    Camera                              &cam_;
    cv::Mat                             frame_;

    QMutex                              mutex_;
    QString                             log_;

    int                                 trigCode_;

    EventContainer                      events_, preEvents_;
    Settings                            settings_;
    MotionDetector                      motionDetector_;
};

#endif /* CAMERATHREAD_H_ */
