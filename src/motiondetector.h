#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <QObject>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <memory>
#include "eventcontainer.h"
#include "event.h"
#include "settings.h"

/*This class detects movement between subsequent frames. It stores a frame of the
background and then detects the hands of the subject by substracting the background from
the frame with hands included. The frame with the background substracted is converted to
a black and white image from which the centroid of the hands can be calculated when tracking
of the hands starts. By inspecting the location of the centoid, movement can be detected.
*/

class MotionDetector : public QObject
{
    Q_OBJECT
public:
    MotionDetector();

    bool            movementDetected(const cv::Mat &frame);
    void            startTracking(EventPtr ev);
    void            changeMovementFrameColor(bool);

    int             getEventTrigCode() const;
    QString         getEventLog() const;

    QPixmap         movementPixmap();

private:
    cv::Mat         prev_, current_, next_, result_, movement_;
    cv::Point       centroid_;

    bool            isTracking_;
    bool            color_;

    EventPtr        event_;
    EventContainer  events_;
};

#endif // MOTIONDETECTOR_H
