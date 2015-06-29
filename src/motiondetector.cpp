#include <iostream>
#include "common.h"
#include "motiondetector.h"

MotionDetector::MotionDetector() : isTracking_(false), event_(nullptr)
{
    Settings settings;
    sensitivity_ = settings.movementSensitivity;
}

MotionDetector::~MotionDetector()
{
    if(event_ != nullptr)
        delete event_;
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

    if(isTracking_) {
        //Calculate the distance between the centroids of the current frame and the frame when tracking started
        //If distance is big enough, interpret it as movement
        double norm = cv::norm(centroid_-getCentroid(fore_));
        if(norm > sensitivity_) {
            emit movementDetected(event_->getTrigCode());
            delete event_;
            event_ = nullptr;
            isTracking_ = false;
        }
    }
}

const QImage& MotionDetector::handsImage()
{
    handsImage_ =  QImage(fore_.data, fore_.cols, fore_.rows, fore_.step, QImage::Format_Indexed8);
    return handsImage_;
}

void MotionDetector::startTracking(Event* ev)
{
    centroid_ = getCentroid(fore_);
    event_ = ev;
    isTracking_ = true;
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
