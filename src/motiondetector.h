#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <QObject>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <memory>
#include "eventcontainer.h"
#include "event.h"
#include "settings.h"

/*This class detects movement between subsequent frames. It stores three frames:
 * previous, current and next, and detects motion using "differential images"
 * method https://blog.cedric.ws/opencv-simple-motion-detection. This class can
 * also emit a QPixmpap of either the original image with the movement
 * highlighted with a red rectangle, or a black-and-white image with the
 * movement shown as white pixels. The emitted pixmap can then be drawn to
 * MotionDialog.
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
