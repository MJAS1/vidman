#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <QObject>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <memory>
#include "event.h"
#include "settings.h"

/*This class detects movement between subsequent frames. It stores a frame of the
background and then detects the hands of the subject by substracting the background from
the frame with hands included. The frame with the background substracted is converted to
a black and white image from which the centroid of the hands can be calculated when tracking
of the hands starts. By inspecting the location of the centoid, movement can be detected.
*/

using std::unique_ptr;

class MotionDetector : public QObject
{
    Q_OBJECT
public:
    MotionDetector();

    void            updateBackground(const cv::Mat &frame);
    bool            movementDetected(const cv::Mat &frame);
    void            startTracking(Event* ev);

    int             getEventTrigCode() const;
    QString         getEventLog() const;

    QPixmap         foregroundPixmap() const;

private:
    cv::Point   getCentroid(const cv::Mat &frame) const;

    cv::Mat     back_, fore_;
    cv::Point   centroid_;

    int         sensitivity_;
    bool        isTracking_;

    unique_ptr<Event>      event_;
};

#endif // MOTIONDETECTOR_H
