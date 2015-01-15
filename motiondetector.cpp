#include "motiondetector.h"

MotionDetector::MotionDetector()
{
}

void MotionDetector::updateBackground(const cv::Mat &frame)
{
    back = frame.clone();
}

void MotionDetector::updateFrame(const cv::Mat &frame)
{
    cv::absdiff(frame, back, fore);
    cv::cvtColor(fore, fore, CV_BGR2GRAY);
    cv::threshold(fore, fore, 100, 0xff, CV_THRESH_BINARY);
    cv::erode(fore,fore,cv::Mat());
    cv::dilate(fore,fore,cv::Mat());
}

void MotionDetector::startTracking()
{
    centroid = getCentroid(fore);
}

bool MotionDetector::movementDetected() const
{
    //Calculate the distance between the centroids of the current frame and the frame when tracking started
    //If distance is big enough, interpret it as movement
    double norm = cv::norm(centroid-getCentroid(fore));
    if(norm > 15)
    {
        return true;
    }

    return false;
}

cv::Point MotionDetector::getCentroid(const cv::Mat &frame) const
{
    cv::Point Coord;
    cv::Moments mm = cv::moments(frame,false);
    double moment10 = mm.m10;
    double moment01 = mm.m01;
    double moment00 = mm.m00;
    Coord.x = int(moment10 / moment00);
    Coord.y = int(moment01 / moment00);
    return Coord;
}
