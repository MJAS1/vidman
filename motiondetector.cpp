#include "motiondetector.h"

MotionDetector::MotionDetector()
{
    Settings settings;
    sensitivity_ = settings.movementSensitivity;
}

void MotionDetector::updateBackground(const cv::Mat &frame)
{
    back_ = frame.clone();
}

void MotionDetector::updateFrame(const cv::Mat &frame)
{
    cv::absdiff(frame, back_, fore_);
    cv::cvtColor(fore_, fore_, CV_BGR2GRAY);
    cv::threshold(fore_, fore_, 100, 0xff, CV_THRESH_BINARY);
    cv::erode(fore_,fore_,cv::Mat());
    cv::dilate(fore_,fore_,cv::Mat());
}

void MotionDetector::startTracking()
{
    centroid_ = getCentroid(fore_);
}

bool MotionDetector::movementDetected() const
{
    //Calculate the distance between the centroids of the current frame and the frame when tracking started
    //If distance is big enough, interpret it as movement
    double norm = cv::norm(centroid_-getCentroid(fore_));
    if(norm > sensitivity_)
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
