#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

/*This class detects movement between subsequent frames. It stores a frame of the
background and then detects the hands of the subject by substracting the background from
the frame with hands included. The frame with the background substracted is converted to
a black and white image from which the centroid of the hands can be calculated when tracking
of the hands starts. By inspecting the location of the centoid, movement can be detected.
*/

class MotionDetector
{
public:
    MotionDetector();

    void updateBackground(const cv::Mat &frame);
    void updateFrame(const cv::Mat &frame);
    void startTracking();

    bool movementDetected() const;

private:

    cv::Point getCentroid(const cv::Mat &frame) const;

    cv::Mat   back_, fore_;
    cv::Point centroid_;

};

#endif // MOTIONDETECTOR_H
