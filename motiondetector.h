#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <cv.h>
#include <highgui.h>

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

    void updateBackground(cv::Mat &frame);
    void updateFrame(cv::Mat &frame);
    void startTracking();

    bool movementDetected();

    cv::Mat getFore() const;

private:

    cv::Point getCentroid(cv::Mat &frame);

    cv::Mat   back, fore;
    cv::Point centroid;

};

#endif // MOTIONDETECTOR_H
