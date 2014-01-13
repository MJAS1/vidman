#include "motiondetector.h"

MotionDetector::MotionDetector()
{
}

void MotionDetector::updateBackground(cv::Mat frame)
{
    back = frame;
}

void MotionDetector::updateFrame(cv::Mat frame)
{

    cv::absdiff(frame, back, fore);
    cv::cvtColor(fore, fore, CV_BGR2GRAY);
    cv::threshold(fore, fore, 100, 0xff, CV_THRESH_BINARY);
    cv::erode(fore,fore,cv::Mat());
    cv::dilate(fore,fore,cv::Mat());

    cv::cvtColor(frame, frame, CV_BGR2RGB);
}

void MotionDetector::startTracking()
{
    centroid = getCentroid(fore);
}

bool MotionDetector::movementDetected()
{
    //Calculate the distance between the centroids of the current frame and the frame when tracking started
    //If distance is big enough, interpret it as movement
    double norm = cv::norm(centroid-getCentroid(fore));
    if(norm > 10)
    {
        return true;
    }

    return false;
}

cv::Point MotionDetector::getCentroid(cv::Mat frame)
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

cv::Mat MotionDetector::getFore() const
{
    return fore;
}
